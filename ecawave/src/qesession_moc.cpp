/****************************************************************************
** QESession meta object code from reading C++ file 'qesession.h'
**
** Created: Mon Apr 3 05:12:25 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.7 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QESession
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qesession.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QESession::className() const
{
    return "QESession";
}

QMetaObject *QESession::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QESession(&QESession::staticMetaObject);

#endif

void QESession::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QESession","QWidget");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QESession::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QESession",s);
}

void QESession::staticMetaObject()
{
    if ( metaObj )
	return;
    QWidget::staticMetaObject();
#else

    QWidget::initMetaObject();
#endif

    typedef void(QESession::*m1_t0)();
    typedef void(QESession::*m1_t1)();
    typedef void(QESession::*m1_t2)();
    typedef void(QESession::*m1_t3)();
    typedef void(QESession::*m1_t4)();
    typedef void(QESession::*m1_t5)();
    typedef void(QESession::*m1_t6)();
    typedef void(QESession::*m1_t7)();
    typedef void(QESession::*m1_t8)();
    typedef void(QESession::*m1_t9)();
    typedef void(QESession::*m1_t10)();
    typedef void(QESession::*m1_t11)();
    typedef void(QESession::*m1_t12)();
    typedef void(QESession::*m1_t13)();
    m1_t0 v1_0 = Q_AMPERSAND QESession::new_session;
    m1_t1 v1_1 = Q_AMPERSAND QESession::new_file;
    m1_t2 v1_2 = Q_AMPERSAND QESession::open_file;
    m1_t3 v1_3 = Q_AMPERSAND QESession::save_event;
    m1_t4 v1_4 = Q_AMPERSAND QESession::save_as_event;
    m1_t5 v1_5 = Q_AMPERSAND QESession::play_event;
    m1_t6 v1_6 = Q_AMPERSAND QESession::stop_event;
    m1_t7 v1_7 = Q_AMPERSAND QESession::effect_event;
    m1_t8 v1_8 = Q_AMPERSAND QESession::copy_event;
    m1_t9 v1_9 = Q_AMPERSAND QESession::paste_event;
    m1_t10 v1_10 = Q_AMPERSAND QESession::cut_event;
    m1_t11 v1_11 = Q_AMPERSAND QESession::debug_event;
    m1_t12 v1_12 = Q_AMPERSAND QESession::position_update;
    m1_t13 v1_13 = Q_AMPERSAND QESession::update_wave_data;
    QMetaData *slot_tbl = QMetaObject::new_metadata(14);
    slot_tbl[0].name = "new_session()";
    slot_tbl[1].name = "new_file()";
    slot_tbl[2].name = "open_file()";
    slot_tbl[3].name = "save_event()";
    slot_tbl[4].name = "save_as_event()";
    slot_tbl[5].name = "play_event()";
    slot_tbl[6].name = "stop_event()";
    slot_tbl[7].name = "effect_event()";
    slot_tbl[8].name = "copy_event()";
    slot_tbl[9].name = "paste_event()";
    slot_tbl[10].name = "cut_event()";
    slot_tbl[11].name = "debug_event()";
    slot_tbl[12].name = "position_update()";
    slot_tbl[13].name = "update_wave_data()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl[5].ptr = *((QMember*)&v1_5);
    slot_tbl[6].ptr = *((QMember*)&v1_6);
    slot_tbl[7].ptr = *((QMember*)&v1_7);
    slot_tbl[8].ptr = *((QMember*)&v1_8);
    slot_tbl[9].ptr = *((QMember*)&v1_9);
    slot_tbl[10].ptr = *((QMember*)&v1_10);
    slot_tbl[11].ptr = *((QMember*)&v1_11);
    slot_tbl[12].ptr = *((QMember*)&v1_12);
    slot_tbl[13].ptr = *((QMember*)&v1_13);
    typedef void(QESession::*m2_t0)(const string&);
    m2_t0 v2_0 = Q_AMPERSAND QESession::filename_changed;
    QMetaData *signal_tbl = QMetaObject::new_metadata(1);
    signal_tbl[0].name = "filename_changed(const string&)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = QMetaObject::new_metaobject(
	"QESession", "QWidget",
	slot_tbl, 14,
	signal_tbl, 1 );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL filename_changed
void QESession::filename_changed( const string& t0 )
{
    // No builtin function for signal parameter type const string&
    QConnectionList *clist = receivers("filename_changed(const string&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(const string&);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}
