/****************************************************************************
** QEFile meta object code from reading C++ file 'qefile.h'
**
** Created: Tue Mar 7 00:35:32 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.3 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEFile
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qefile.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEFile::className() const
{
    return "QEFile";
}

QMetaObject *QEFile::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QEFile(&QEFile::staticMetaObject);

#endif

void QEFile::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEFile","QWidget");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QEFile::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEFile",s);
}

void QEFile::staticMetaObject()
{
    if ( metaObj )
	return;
    QWidget::staticMetaObject();
#else

    QWidget::initMetaObject();
#endif

    typedef void(QEFile::*m1_t0)();
    typedef void(QEFile::*m1_t1)(const ECA_AUDIO_FORMAT&);
    typedef void(QEFile::*m1_t2)(long int);
    typedef void(QEFile::*m1_t3)(long int,long int);
    typedef void(QEFile::*m1_t4)(int,int);
    typedef void(QEFile::*m1_t5)();
    typedef void(QEFile::*m1_t6)();
    typedef void(QEFile::*m1_t7)();
    m1_t0 v1_0 = Q_AMPERSAND QEFile::update_wave_form_data;
    m1_t1 v1_1 = Q_AMPERSAND QEFile::set_audio_format;
    m1_t2 v1_2 = Q_AMPERSAND QEFile::current_position;
    m1_t3 v1_3 = Q_AMPERSAND QEFile::visible_area;
    m1_t4 v1_4 = Q_AMPERSAND QEFile::mark_area_relative;
    m1_t5 v1_5 = Q_AMPERSAND QEFile::unmark;
    m1_t6 v1_6 = Q_AMPERSAND QEFile::zoom_to_marked;
    m1_t7 v1_7 = Q_AMPERSAND QEFile::zoom_out;
    QMetaData *slot_tbl = QMetaObject::new_metadata(8);
    slot_tbl[0].name = "update_wave_form_data()";
    slot_tbl[1].name = "set_audio_format(const ECA_AUDIO_FORMAT&)";
    slot_tbl[2].name = "current_position(long int)";
    slot_tbl[3].name = "visible_area(long int,long int)";
    slot_tbl[4].name = "mark_area_relative(int,int)";
    slot_tbl[5].name = "unmark()";
    slot_tbl[6].name = "zoom_to_marked()";
    slot_tbl[7].name = "zoom_out()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl[5].ptr = *((QMember*)&v1_5);
    slot_tbl[6].ptr = *((QMember*)&v1_6);
    slot_tbl[7].ptr = *((QMember*)&v1_7);
    typedef void(QEFile::*m2_t0)();
    typedef void(QEFile::*m2_t1)(ECA_AUDIO_TIME);
    typedef void(QEFile::*m2_t2)(ECA_AUDIO_TIME,ECA_AUDIO_TIME);
    typedef void(QEFile::*m2_t3)(ECA_AUDIO_TIME,ECA_AUDIO_TIME);
    m2_t0 v2_0 = Q_AMPERSAND QEFile::selection_changed;
    m2_t1 v2_1 = Q_AMPERSAND QEFile::current_position_changed;
    m2_t2 v2_2 = Q_AMPERSAND QEFile::visible_area_changed;
    m2_t3 v2_3 = Q_AMPERSAND QEFile::marked_area_changed;
    QMetaData *signal_tbl = QMetaObject::new_metadata(4);
    signal_tbl[0].name = "selection_changed()";
    signal_tbl[1].name = "current_position_changed(ECA_AUDIO_TIME)";
    signal_tbl[2].name = "visible_area_changed(ECA_AUDIO_TIME,ECA_AUDIO_TIME)";
    signal_tbl[3].name = "marked_area_changed(ECA_AUDIO_TIME,ECA_AUDIO_TIME)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    signal_tbl[1].ptr = *((QMember*)&v2_1);
    signal_tbl[2].ptr = *((QMember*)&v2_2);
    signal_tbl[3].ptr = *((QMember*)&v2_3);
    metaObj = QMetaObject::new_metaobject(
	"QEFile", "QWidget",
	slot_tbl, 8,
	signal_tbl, 4 );
}

// SIGNAL selection_changed
void QEFile::selection_changed()
{
    activate_signal( "selection_changed()" );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL current_position_changed
void QEFile::current_position_changed( ECA_AUDIO_TIME t0 )
{
    // No builtin function for signal parameter type ECA_AUDIO_TIME
    QConnectionList *clist = receivers("current_position_changed(ECA_AUDIO_TIME)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(ECA_AUDIO_TIME);
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

// SIGNAL visible_area_changed
void QEFile::visible_area_changed( ECA_AUDIO_TIME t0, ECA_AUDIO_TIME t1 )
{
    // No builtin function for signal parameter type ECA_AUDIO_TIME,ECA_AUDIO_TIME
    QConnectionList *clist = receivers("visible_area_changed(ECA_AUDIO_TIME,ECA_AUDIO_TIME)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(ECA_AUDIO_TIME);
    typedef RT1 *PRT1;
    typedef void (QObject::*RT2)(ECA_AUDIO_TIME,ECA_AUDIO_TIME);
    typedef RT2 *PRT2;
    RT0 r0;
    RT1 r1;
    RT2 r2;
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
	    case 2:
		r2 = *((PRT2)(c->member()));
		(object->*r2)(t0, t1);
		break;
	}
    }
}

// SIGNAL marked_area_changed
void QEFile::marked_area_changed( ECA_AUDIO_TIME t0, ECA_AUDIO_TIME t1 )
{
    // No builtin function for signal parameter type ECA_AUDIO_TIME,ECA_AUDIO_TIME
    QConnectionList *clist = receivers("marked_area_changed(ECA_AUDIO_TIME,ECA_AUDIO_TIME)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(ECA_AUDIO_TIME);
    typedef RT1 *PRT1;
    typedef void (QObject::*RT2)(ECA_AUDIO_TIME,ECA_AUDIO_TIME);
    typedef RT2 *PRT2;
    RT0 r0;
    RT1 r1;
    RT2 r2;
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
	    case 2:
		r2 = *((PRT2)(c->member()));
		(object->*r2)(t0, t1);
		break;
	}
    }
}
