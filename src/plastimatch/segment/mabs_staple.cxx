/* -----------------------------------------------------------------------
 *    See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
 *       ----------------------------------------------------------------------- */
#include "plmsegment_config.h"

#include "itkImage.h"
#include <itkSTAPLEImageFilter.h>

#include "mabs_staple.h"

Mabs_staple::Mabs_staple() {

    this->foreground_val = 1;
    this->confidence_weight = 1.0;

}


Mabs_staple::~Mabs_staple() {

    this->structures.clear();

}


void
Mabs_staple::add_input_structure(Plm_image::Pointer& structure) {

    this->structures.push_back(structure);

}

void
Mabs_staple::set_confidence_weight(float confidence_weight) {

    this->confidence_weight = confidence_weight;

}

void
Mabs_staple::run() {

    typedef itk::Image<unsigned char, 3 > InputImageType;
    typedef itk::Image<float, 3 > OutputImageType;

    typedef itk::STAPLEImageFilter< InputImageType, OutputImageType > StapleType;
    StapleType::Pointer staple = StapleType::New();

    std::list<Plm_image::Pointer>::iterator stru_it;
    int i;
    for (stru_it = this->structures.begin(), i=0;
         stru_it != this->structures.end(); stru_it++, i++) {

        staple->SetInput(i, (*stru_it)->itk_uchar());
    }

    staple->SetForegroundValue(this->foreground_val);
    if (this->confidence_weight != 1.0) staple->SetConfidenceWeight((double) this->confidence_weight);
    staple->Update();

    this->output_img = Plm_image::New(staple->GetOutput());

}

