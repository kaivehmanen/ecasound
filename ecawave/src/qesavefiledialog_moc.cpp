/****************************************************************************
** QESaveFileDialog meta object code from reading C++ file 'qesavefiledialog.h'
**
** Created: Sat Mar 11 02:26:57 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.4 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QESaveFileDialog
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
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


#if QT_VERSION >= 199
static QMetaObjectInit init_QESaveFileDialog(&QESaveFileDialog::staticMetaObject);

#endif

void QESaveFileDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("QESaveFileDialog","QDialog");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QESaveFileDialog::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QESaveFileDialog",s);
}

void QESaveFileDialog::staticMetaObject()
{
    if ( metaObj )
	return;
    QDialog::staticMetaObject();
#else

    QDialog::initMetaObject();
#endif

    metaObj = QMetaObject::new_metaobject(
	"QESaveFileDialog", "QDialog",
	0, 0,
	0, 0 );
}
