/****************************************************************************
** QEChainopInput meta object code from reading C++ file 'qechainopinput.h'
**
** Created: Sat Jan 22 23:24:28 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.1 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEChainopInput
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qechainopinput.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEChainopInput::className() const
{
    return "QEChainopInput";
}

QMetaObject *QEChainopInput::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QEChainopInput(&QEChainopInput::staticMetaObject);

#endif

void QEChainopInput::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEChainopInput","QWidget");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QEChainopInput::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEChainopInput",s);
}

void QEChainopInput::staticMetaObject()
{
    if ( metaObj )
	return;
    QWidget::staticMetaObject();
#else

    QWidget::initMetaObject();
#endif

    typedef void(QEChainopInput::*m1_t0)(int);
    typedef void(QEChainopInput::*m1_t1)();
    m1_t0 v1_0 = Q_AMPERSAND QEChainopInput::update_chainop;
    m1_t1 v1_1 = Q_AMPERSAND QEChainopInput::set_parameters;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    slot_tbl[0].name = "update_chainop(int)";
    slot_tbl[1].name = "set_parameters()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    metaObj = QMetaObject::new_metaobject(
	"QEChainopInput", "QWidget",
	slot_tbl, 2,
	0, 0 );
}
