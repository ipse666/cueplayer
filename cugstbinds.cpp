#include "cugstbinds.h"

CuGstBinds::CuGstBinds(QObject *parent) :
    QObject(parent)
{
}

bool CuGstBinds::checkElement(QString s)
{
	GstElementFactory *srcfactory;

	srcfactory = gst_element_factory_find (s.toUtf8().data());

	return (bool)srcfactory;
}
