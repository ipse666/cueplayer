#include "cugstbinds.h"

CuGstBinds::CuGstBinds(QObject *parent) :
    QObject(parent)
{
	// Костыль для неюникодовых локалей
	localFileNamesEncoder = QTextCodec::codecForLocale()->makeEncoder();
}

bool CuGstBinds::checkElement(QString s)
{
	GstElementFactory *srcfactory;

	srcfactory = gst_element_factory_find (localFileNamesEncoder->fromUnicode(s).data());

	return (bool)srcfactory;
}
