/****************************************************************************
** QEChainopInput meta object code from reading C++ file 'qechainopinput.h'
**
** Created: Sat Feb 19 00:56:26 2000
**      by: The Qt MOC ($Id: qechainopinput_moc.cpp,v 1.2 2000-02-21 23:21:48 kaiv Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEChainopInput
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 7
#elif Q_MOC_OUTPUT_REVISION != 7
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

void QEChainopInput::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEChainopInput","QWidget");
    (void) staticMetaObject();
}

QString QEChainopInput::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEChainopInput",s);
}

QMetaObject* QEChainopInput::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QWidget::staticMetaObject();
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
	0, 0,
	0, 0,
	0, 0,
	0, 0 );
    return metaObj;
}
