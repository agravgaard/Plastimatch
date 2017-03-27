/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcmtk_series_map_h_
#define _dcmtk_series_map_h_

#include <map>

class Dcmtk_series;

/*! Map from SeriesInstanceUID to Dcmtk_series */
typedef std::map<std::string, Dcmtk_series*> Dcmtk_series_map;
typedef std::pair<std::string, Dcmtk_series*> Dcmtk_series_map_pair;

#endif
