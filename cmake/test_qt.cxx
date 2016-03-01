#include <QString>

QString foo ()
{
    return QString ("a") + QString ("b");
}

int 
main (int argc, char* argv[])
{
    QString q1 ("Hello world");

    /* The below test fails on VS 2013 when attempting to link against 
       QT built with VS 2008 */
    QString q2;
    q2 = foo ();
}
