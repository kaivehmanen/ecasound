/****************************************************************************
** QEAcceptInput meta object code from reading C++ file 'qeacceptinput.h'
**
** Created: Sat Feb 19 00:55:04 2000
**      by: The Qt MOC ($Id: qeacceptinput_moc.cpp,v 1.2 2000-02-21 23:21:47 kaiv Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEAcceptInput
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 7
#elif Q_MOC_OUTPUT_REVISION != 7
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qeacceptinput.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEAcceptInput::className() const
{
    return "QEAcceptInput";
}

QMetaObject *QEAcceptInput::metaObj = 0;

void QEAcceptInput::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEAcceptInput","QWidget");
    (void) staticMetaObject();
}

QString QEAcceptInput::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEAcceptInput",s);
}

QMetaObject* QEAcceptInput::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QWidget::staticMetaObject();
    typedef void(QEAcceptInput::*m1_t0)();
    typedef void(QEAcceptInput::*m1_t1)();
    m1_t0 v1_0 = Q_AMPERSAND QEAcceptInput::accept;
    m1_t1 v1_1 = Q_AMPERSAND QEAcceptInput::reject;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    slot_tbl[0].name = "accept()";
    slot_tbl[1].name = "reject()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    typedef void(QEAcceptInput::*m2_t0)();
    typedef void(QEAcceptInput::*m2_t1)();
    m2_t0 v2_0 = Q_AMPERSAND QEAcceptInput::ok;
    m2_t1 v2_1 = Q_AMPERSAND QEAcceptInput::cancel;
    QMetaData *signal_tbl = QMetaObject::new_metadata(2);
    signal_tbl[0].name = "ok()";
    signal_tbl[1].name = "cancel()";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    signal_tbl[1].ptr = *((QMember*)&v2_1);
    metaObj = QMetaObject::new_metaobject(
	"QEAcceptInput", "QWidget",
	slot_tbl, 2,
	signal_tbl, 2,
	0, 0,
	0, 0,
	0, 0 );
    return metaObj;
}

// SIGNAL ok
void QEAcceptInput::ok()
{
    activate_signal( "ok()" );
}

// SIGNAL cancel
void QEAcceptInput::cancel()
{
    activate_signal( "cancel()" );
}
