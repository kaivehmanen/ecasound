/****************************************************************************
** QEChainopEvent meta object code from reading C++ file 'qechainopevent.h'
**
** Created: Sat Mar 11 02:27:08 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.4 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEChainopEvent
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
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


#if QT_VERSION >= 199
static QMetaObjectInit init_QEChainopEvent(&QEChainopEvent::staticMetaObject);

#endif

void QEChainopEvent::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("QEChainopEvent","QDialog");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QEChainopEvent::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEChainopEvent",s);
}

void QEChainopEvent::staticMetaObject()
{
    if ( metaObj )
	return;
    QDialog::staticMetaObject();
#else

    QDialog::initMetaObject();
#endif

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
	signal_tbl, 1 );
}

// SIGNAL finished
void QEChainopEvent::finished()
{
    activate_signal( "finished()" );
}
