

#include "oraQStringTools.h"


#include <sstream>

#include "oraStringTools.h"

// Forward declarations
#include <QDateTime>

namespace ora
{


int
MillisecondsBetweenQDateTimes(QDateTime *dt1, QDateTime *dt2)
{
  if (!dt1 || !dt2)
    return 0;

  int ms = 0;

  ms = dt1->time().msecsTo(dt2->time()); // basic (for same day date/times)

  if (dt1->date() < dt2->date()) // different days
  {
    QDateTime dt = *dt2; // copy
    while (dt.date() > dt1->date())
    {
      dt = dt.addDays(-1);
      ms += 86400000;
    }
  }

  return ms;
}


QString
MillisecondsToTimeSpanQString(int ms, const QString msString,
    const QString sString, const QString mString, const QString hString,
    const QString dString)
{
  if (ms < 0)
    return "";

  int s, m, h, d;

  // decompose the time value
  // days
  d = 0;
  while (ms >= 86400000)
  {
    ms -= 86400000;
    d++;
  }
  // hours
  h = 0;
  while (ms >= 3600000)
  {
    ms -= 3600000;
    h++;
  }
  // minutes
  m = 0;
  while (ms >= 60000)
  {
    ms -= 60000;
    m++;
  }
  // seconds
  s = 0;
  while (ms >= 1000)
  {
    ms -= 1000;
    s++;
  }

  // compose the string
  std::ostringstream os;

  if (d > 0)
    os << d << dString.toStdString();
  if (h > 0)
    os << " " << h << hString.toStdString();
  if (m > 0)
    os << " " << m << mString.toStdString();
  if (s > 0)
    os << " " << s << sString.toStdString();
  if (ms > 0)
    os << " " << ms << msString.toStdString();

  return QString::fromStdString(TrimF(os.str()));
}

std::string
MillisecondsToTimeSpanStdString(int ms, const QString msString,
    const QString sString, const QString mString, const QString hString,
    const QString dString)
{
  return MillisecondsToTimeSpanQString(ms, msString, sString, mString,
      hString, dString).toStdString();
}


}

