/****************************************************************************
** QEButtonRow meta object code from reading C++ file 'qebuttonrow.h'
**
** Created: Mon Dec 20 04:24:33 1999
**      by: The Qt Meta Object Compiler ($Revision: 1.1 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEButtonRow
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "qebuttonrow.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEButtonRow::className() const
{
    return "QEButtonRow";
}

QMetaObject *QEButtonRow::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QEButtonRow(&QEButtonRow::staticMetaObject);

#endif

void QEButtonRow::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEButtonRow","QWidget");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QEButtonRow::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEButtonRow",s);
}

void QEButtonRow::staticMetaObject()
{
    if ( metaObj )
	return;
    QWidget::staticMetaObject();
#else

    QWidget::initMetaObject();
#endif

    metaObj = QMetaObject::new_metaobject(
	"QEButtonRow", "QWidget",
	0, 0,
	0, 0 );
}
