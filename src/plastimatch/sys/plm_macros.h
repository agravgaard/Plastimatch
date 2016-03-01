/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_macros_h_
#define _plm_macros_h_

/* Nb. Doxygen values are substituted in PREDEFINED value of Doxyfile.in */

#define PLM_GET_SET(type, name)                              \
    const type& get_##name () const;                         \
    void set_##name (const type&)

#define PLM_GET(type, name)                                  \
    type get_##name () const

#define PLM_GET_CR(type, name)                               \
    const type& get_##name () const

#define PLM_SET(type, name)                                  \
    void set_##name (type)

#define PLM_SET_CR(type, name)                               \
    void set_##name (const type&)

#endif
