/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler_warnings.h"
#include "landmark_warp.h"
#include "plm_math.h"
#include "raw_pointset.h"
#include "rbf_cluster.h"

//k-means++ clustering algorithm to separate landmarks into user-specified number of clusters
void
rbf_cluster_kmeans_plusplus(Landmark_warp *lw)
{
    int num_landmarks = lw->m_fixed_landmarks.get_count();
    int num_clusters = lw->num_clusters;
    float *mx, *my, *mz;
    float *D, *DD;
    int i;
    float r, d, dmin = FLT_MAX;
    int clust_id = 0;
    int kcurrent, count, reassigned, iter_count =0;
	
    mx = (float *)malloc(num_clusters*sizeof(float));
    my = (float *)malloc(num_clusters*sizeof(float));
    mz = (float *)malloc(num_clusters*sizeof(float));
    D  = (float *)malloc(num_landmarks*sizeof(float));
    DD = (float *)malloc(num_landmarks*sizeof(float));
		
    for(i=0;i<num_landmarks;i++) lw->cluster_id[i]=-1;

    //kmeans++ initialization
    i = (int)((double)rand()/RAND_MAX*(num_landmarks-1.));
#if defined (commentout)
    mx[0]=lw->m_fixed_landmarks->points[i*3+0];
    my[0]=lw->m_fixed_landmarks->points[i*3+1]; 
    mz[0]=lw->m_fixed_landmarks->points[i*3+2];
#endif
    mx[0] = lw->m_fixed_landmarks.point(i,0);
    my[0] = lw->m_fixed_landmarks.point(i,1);
    mz[0] = lw->m_fixed_landmarks.point(i,2);
    kcurrent=1;

    do {
	for(i=0;i<num_landmarks;i++) {
	    for(int j=0;j<kcurrent;j++) {
		d = (lw->m_fixed_landmarks.point(i,0)-mx[j])
                    * (lw->m_fixed_landmarks.point(i,0)-mx[j])
		    + (lw->m_fixed_landmarks.point(i,1)-my[j])
		    * (lw->m_fixed_landmarks.point(i,1)-my[j]) 
		    + (lw->m_fixed_landmarks.point(i,2)-mz[j])
		    * (lw->m_fixed_landmarks.point(i,2)-mz[j]);
		if (j==0) { dmin=d; }
		if (d<=dmin) { D[i]=dmin; }
	    }
	}

	//DD is a normalized cumulative sum of D
	d=0;
	for(i=0;i<num_landmarks;i++) d+=D[i];
	for(i=0;i<num_landmarks;i++) D[i]/=d;
	d=0;
	for(i=0;i<num_landmarks;i++) { d+=D[i]; DD[i]=d; }

	// randomly select j with probability proportional to D
	int j = 0;
	r = ((double)rand())/RAND_MAX;
	for(i=0;i<num_landmarks;i++) {
	    if ( i==0 && r<=DD[i] ) j = 0;
	    if ( i>0  && DD[i-1]<r && r<=DD[i] ) j = i;
	}

	mx[kcurrent] = lw->m_fixed_landmarks.point(j,0); 
	my[kcurrent] = lw->m_fixed_landmarks.point(j,1); 
	mz[kcurrent] = lw->m_fixed_landmarks.point(j,2);
	kcurrent++;

    } while(kcurrent < num_clusters);


    //standard k-means algorithm
    do {
	reassigned = 0;

	// assign
	for(i=0;i<num_landmarks;i++) {
	    for(int j=0;j<num_clusters;j++) {
		d =  (lw->m_fixed_landmarks.point(i,0)-mx[j])
		    *(lw->m_fixed_landmarks.point(i,0)-mx[j]) + 
		    (lw->m_fixed_landmarks.point(i,1)-my[j])
		    *(lw->m_fixed_landmarks.point(i,1)-my[j]) + 
		    (lw->m_fixed_landmarks.point(i,2)-mz[j])
		    *(lw->m_fixed_landmarks.point(i,2)-mz[j]);
		if (j==0) { dmin=d; clust_id = 0; }
		if (d<=dmin) { dmin =d; clust_id = j; }
	    }
    
	    if ( lw->cluster_id[i] != clust_id) reassigned = 1;
	    lw->cluster_id[i] = clust_id;
	}

	// calculate new means
	for(int j=0;j<num_clusters;j++) {
	    mx[j]=0; my[j]=0; mz[j]=0; count=0;
	    for(i=0;i<num_landmarks;i++) {
		if (lw->cluster_id[i]==j) { 
		    mx[j]+=lw->m_fixed_landmarks.point(i,0); 
		    my[j]+=lw->m_fixed_landmarks.point(i,1); 
		    mz[j]+=lw->m_fixed_landmarks.point(i,2); 
		    count++; 
		}
	    }
	    mx[j]/=count; my[j]/=count; mz[j]/=count;
	}

	iter_count++;

    } while(reassigned && (iter_count<10000));

    fprintf(stderr,"iter count %d\n", iter_count);

    free(D);
    free(DD);
    free(mx);
    free(my);
    free(mz);
}

//calculate adaptive radius of each RBF
void
rbf_cluster_find_adapt_radius(Landmark_warp *lw)
{
    int i,j,k, count;
    int num_clusters = lw->num_clusters;
    int num_landmarks = lw->m_fixed_landmarks.get_count();
    float d, D, dmax;
    float *d_nearest_neighb;

// NB what to do if there is just one landmark in a cluster??

    for(k=0; k<num_clusters; k++) {
	int num_landm_in_clust = 0; 
	for(i=0; i<num_landmarks; i++)
	{if (lw->cluster_id[i]==k)
                num_landm_in_clust++;}
	if (num_landm_in_clust>1){
            D = 0; count = 0; dmax=-1;
            for(i=0; i<num_landmarks; i++) {
                for(j=i; j<num_landmarks; j++) {
                    if ( lw->cluster_id[i] == k && lw->cluster_id[j] == k  && j != i ) {
                        d = (lw->m_fixed_landmarks.point(i,0)-lw->m_fixed_landmarks.point(j,0))
                            *(lw->m_fixed_landmarks.point(i,0)-lw->m_fixed_landmarks.point(j,0)) + 
                            (lw->m_fixed_landmarks.point(i,1)-lw->m_fixed_landmarks.point(j,1))
                            *(lw->m_fixed_landmarks.point(i,1)-lw->m_fixed_landmarks.point(j,1)) + 
                            (lw->m_fixed_landmarks.point(i,2)-lw->m_fixed_landmarks.point(j,2))
                            *(lw->m_fixed_landmarks.point(i,2)-lw->m_fixed_landmarks.point(j,2));
                        D  += sqrt(d);
                        if (sqrt(d)>dmax) dmax = sqrt(d);
                        count++;
                    }
                }
            }
            D /= count;	
            D = D ; //a magic number

            printf("nclust %d   nland %d   dmax = %f  D = %f\n", num_clusters, num_landm_in_clust, dmax, D);
    
	
            // single long cluster needs other treatment
//    if ( (num_clusters == 1) && (dmax/(D) > 1.5) ) { 
            if ( (dmax/(D) > 2) ) { 
                printf("long cluster, dmax %f D %f\n", dmax, D); 
                //D = dmax/2.1; 
        
                // radius is the max distance between nearest neighbors
	
                d_nearest_neighb = (float *)malloc(num_landmarks*sizeof(float));
                for(i=0;i<num_landmarks;i++) 
                    if (lw->cluster_id[i]==k)	
                        d_nearest_neighb[i]=1e20;
    
                for(i=0;i<num_landmarks;i++) {
                    for(j=0;j<num_landmarks;j++) {
                        if (i==j) continue;
                        if ( lw->cluster_id[i]!=k || lw->cluster_id[j] !=k ) continue;
                        d = (lw->m_fixed_landmarks.point(i,0)-lw->m_fixed_landmarks.point(j,0))
                            *(lw->m_fixed_landmarks.point(i,0)-lw->m_fixed_landmarks.point(j,0)) + 
                            (lw->m_fixed_landmarks.point(i,1)-lw->m_fixed_landmarks.point(j,1))
                            *(lw->m_fixed_landmarks.point(i,1)-lw->m_fixed_landmarks.point(j,1)) + 
                            (lw->m_fixed_landmarks.point(i,2)-lw->m_fixed_landmarks.point(j,2))
                            *(lw->m_fixed_landmarks.point(i,2)-lw->m_fixed_landmarks.point(j,2));
                        d = sqrt(d);	    
                        if (d<d_nearest_neighb[i]) d_nearest_neighb[i]=d;
                    }
                }
	
                D = d_nearest_neighb[0];
                for(i=0;i<num_landmarks;i++) { 
                    if ( d_nearest_neighb[i]>D && lw->cluster_id[i]==k ) D = d_nearest_neighb[i]; 
                }
    
                free(d_nearest_neighb);
            } // end if dmax/D>...,  long cluster

	} //end if many landmarks in a cluster
	else {D=50;}//clusters of one landmark get a magic radius


        for(i=0; i<num_landmarks; i++) { 
            if (lw->cluster_id[i] == k) lw->adapt_radius[i] = D;
	}

    } // end for k=0..num_clusters
	
    return;
}
