/****************************************************************************
** QESaveFileDialog meta object code from reading C++ file 'qesavefiledialog.h'
**
** Created: Sat Feb 19 00:56:07 2000
**      by: The Qt MOC ($Id: qesavefiledialog_moc.cpp,v 1.2 2000-02-21 23:21:48 kaiv Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QESaveFileDialog
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 7
#elif Q_MOC_OUTPUT_REVISION != 7
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qesavefiledialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QESaveFileDialog::className() const
{
    return "QESaveFileDialog";
}

QMetaObject *QESaveFileDialog::metaObj = 0;

void QESaveFileDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("QESaveFileDialog","QDialog");
    (void) staticMetaObject();
}

QString QESaveFileDialog::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QESaveFileDialog",s);
}

QMetaObject* QESaveFileDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"QESaveFileDialog", "QDialog",
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0 );
    return metaObj;
}
