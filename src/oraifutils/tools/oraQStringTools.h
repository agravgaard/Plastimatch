

// This module contains some general tool-functions for string and
// string-related operations not provided by ANSI-C++ or ITK. It is (partially)
// based on Qt and, therefore, separated from oraStringTools.h.
//
// @see oraStringTools.h
//
// @version 1.0
// @author phil 
#ifndef ORAQSTRINGTOOLS_H_
#define ORAQSTRINGTOOLS_H_

#include <string>

#include <QString>


// Forward declarations
class QDateTime;

namespace ora
{


/**
 * Calculate the time span between two QDateTime specifications (dt2 - dt1) in
 * milliseconds resolution. The time span should not exceed too many days (int
 * range!).
 * @param dt1 date time specification
 * @param dt2 date time specification (must be greater than dt1)
 * @return the number of milliseconds between the specifications
 */
int MillisecondsBetweenQDateTimes(QDateTime *dt1, QDateTime *dt2);


/**
 * Convert a millisecond time span into a structured QString which contains
 * various time part elements. For example, a value of 3500 ms will
 * result in this QString value (in English by default): 3s 500ms.
 * Time parts (in English by default):
 * day (d), hour (h), minute (m), second (s), millisecond (ms).
 * @param ms the time span in milliseconds
 * @param msString (optional, English by default) customized string for
 * millisecond unit
 * @param sString (optional, English by default) customized string for
 * second unit
 * @param mString (optional, English by default) customized string for
 * minute unit
 * @param hString (optional, English by default) customized string for
 * hour unit
 * @param dString (optional, English by default) customized string for
 * day unit
 * @return the converted QString-representation or an empty string if the
 * value could not be converted (e.g. negative time spans)
 */
QString MillisecondsToTimeSpanQString(int ms, const QString msString = "ms",
    const QString sString = "s", const QString mString = "m",
    const QString hString = "h", const QString dString = "d");

/**
 * Convert a millisecond time span into a structured std::string which contains
 * internationalized time part elements.
 * @see ora::MillisecondsToTimeSpanQString()
 */
std::string MillisecondsToTimeSpanStdString(int ms,
    const QString msString = "ms",
    const QString sString = "s", const QString mString = "m",
    const QString hString = "h", const QString dString = "d");


}

#endif /* ORAQSTRINGTOOLS_H_ */

