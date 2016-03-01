/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ise_ontrak_h__
#define __ise_ontrak_h__

OntrakData* ise_ontrak_init (void);
void ise_ontrak_engage_relay (OntrakData* od, int gate_beam, int bright_frame);
void ise_ontrak_shutdown (OntrakData* od);

#endif
