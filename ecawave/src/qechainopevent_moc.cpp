/****************************************************************************
** QEChainopEvent meta object code from reading C++ file 'qechainopevent.h'
**
** Created: Sat Feb 19 00:57:30 2000
**      by: The Qt MOC ($Id: qechainopevent_moc.cpp,v 1.2 2000-02-21 23:21:48 kaiv Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEChainopEvent
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 7
#elif Q_MOC_OUTPUT_REVISION != 7
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qechainopevent.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEChainopEvent::className() const
{
    return "QEChainopEvent";
}

QMetaObject *QEChainopEvent::metaObj = 0;

void QEChainopEvent::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("QEChainopEvent","QDialog");
    (void) staticMetaObject();
}

QString QEChainopEvent::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEChainopEvent",s);
}

QMetaObject* QEChainopEvent::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
    typedef void(QEChainopEvent::*m1_t0)();
    typedef void(QEChainopEvent::*m1_t1)();
    m1_t0 v1_0 = Q_AMPERSAND QEChainopEvent::process;
    m1_t1 v1_1 = Q_AMPERSAND QEChainopEvent::preview;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    slot_tbl[0].name = "process()";
    slot_tbl[1].name = "preview()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    typedef void(QEChainopEvent::*m2_t0)();
    m2_t0 v2_0 = Q_AMPERSAND QEChainopEvent::finished;
    QMetaData *signal_tbl = QMetaObject::new_metadata(1);
    signal_tbl[0].name = "finished()";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = QMetaObject::new_metaobject(
	"QEChainopEvent", "QDialog",
	slot_tbl, 2,
	signal_tbl, 1,
	0, 0,
	0, 0,
	0, 0 );
    return metaObj;
}

// SIGNAL finished
void QEChainopEvent::finished()
{
    activate_signal( "finished()" );
}
