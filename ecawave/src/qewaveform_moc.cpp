/****************************************************************************
** QEWaveForm meta object code from reading C++ file 'qewaveform.h'
**
** Created: Sat Feb 19 00:56:56 2000
**      by: The Qt MOC ($Id: qewaveform_moc.cpp,v 1.2 2000-02-21 23:21:48 kaiv Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEWaveForm
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 7
#elif Q_MOC_OUTPUT_REVISION != 7
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qewaveform.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEWaveForm::className() const
{
    return "QEWaveForm";
}

QMetaObject *QEWaveForm::metaObj = 0;

void QEWaveForm::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEWaveForm","QWidget");
    (void) staticMetaObject();
}

QString QEWaveForm::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEWaveForm",s);
}

QMetaObject* QEWaveForm::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QWidget::staticMetaObject();
    typedef void(QEWaveForm::*m1_t0)(const vector<QEWaveBlock>*);
    typedef void(QEWaveForm::*m1_t1)(int);
    typedef void(QEWaveForm::*m1_t2)(long int);
    typedef void(QEWaveForm::*m1_t3)();
    typedef void(QEWaveForm::*m1_t4)(long int);
    typedef void(QEWaveForm::*m1_t5)(long int);
    typedef void(QEWaveForm::*m1_t6)(int,int);
    typedef void(QEWaveForm::*m1_t7)(bool);
    typedef void(QEWaveForm::*m1_t8)(long int,long int);
    typedef void(QEWaveForm::*m1_t9)();
    typedef void(QEWaveForm::*m1_t10)(const QColor&);
    typedef void(QEWaveForm::*m1_t11)(const QColor&);
    typedef void(QEWaveForm::*m1_t12)(const QColor&);
    typedef void(QEWaveForm::*m1_t13)(const QColor&);
    typedef void(QEWaveForm::*m1_t14)(const QColor&);
    typedef void(QEWaveForm::*m1_t15)(const QColor&);
    typedef void(QEWaveForm::*m1_t16)(const QColor&);
    typedef void(QEWaveForm::*m1_t17)(const QColor&);
    m1_t0 v1_0 = Q_AMPERSAND QEWaveForm::update_wave_blocks;
    m1_t1 v1_1 = Q_AMPERSAND QEWaveForm::current_position_relative;
    m1_t2 v1_2 = Q_AMPERSAND QEWaveForm::current_position;
    m1_t3 v1_3 = Q_AMPERSAND QEWaveForm::repaint_current_position;
    m1_t4 v1_4 = Q_AMPERSAND QEWaveForm::marked_area_begin;
    m1_t5 v1_5 = Q_AMPERSAND QEWaveForm::marked_area_end;
    m1_t6 v1_6 = Q_AMPERSAND QEWaveForm::mark_area_relative;
    m1_t7 v1_7 = Q_AMPERSAND QEWaveForm::toggle_marking;
    m1_t8 v1_8 = Q_AMPERSAND QEWaveForm::visible_area;
    m1_t9 v1_9 = Q_AMPERSAND QEWaveForm::zoom_to_marked;
    m1_t10 v1_10 = Q_AMPERSAND QEWaveForm::set_wave_color;
    m1_t11 v1_11 = Q_AMPERSAND QEWaveForm::set_background_color;
    m1_t12 v1_12 = Q_AMPERSAND QEWaveForm::set_position_color;
    m1_t13 v1_13 = Q_AMPERSAND QEWaveForm::set_marked_color;
    m1_t14 v1_14 = Q_AMPERSAND QEWaveForm::set_marked_background_color;
    m1_t15 v1_15 = Q_AMPERSAND QEWaveForm::set_marked_position_color;
    m1_t16 v1_16 = Q_AMPERSAND QEWaveForm::set_minmax_color;
    m1_t17 v1_17 = Q_AMPERSAND QEWaveForm::set_zeroline_color;
    QMetaData *slot_tbl = QMetaObject::new_metadata(18);
    slot_tbl[0].name = "update_wave_blocks(const vector<QEWaveBlock>*)";
    slot_tbl[1].name = "current_position_relative(int)";
    slot_tbl[2].name = "current_position(long int)";
    slot_tbl[3].name = "repaint_current_position()";
    slot_tbl[4].name = "marked_area_begin(long int)";
    slot_tbl[5].name = "marked_area_end(long int)";
    slot_tbl[6].name = "mark_area_relative(int,int)";
    slot_tbl[7].name = "toggle_marking(bool)";
    slot_tbl[8].name = "visible_area(long int,long int)";
    slot_tbl[9].name = "zoom_to_marked()";
    slot_tbl[10].name = "set_wave_color(const QColor&)";
    slot_tbl[11].name = "set_background_color(const QColor&)";
    slot_tbl[12].name = "set_position_color(const QColor&)";
    slot_tbl[13].name = "set_marked_color(const QColor&)";
    slot_tbl[14].name = "set_marked_background_color(const QColor&)";
    slot_tbl[15].name = "set_marked_position_color(const QColor&)";
    slot_tbl[16].name = "set_minmax_color(const QColor&)";
    slot_tbl[17].name = "set_zeroline_color(const QColor&)";
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
    slot_tbl[14].ptr = *((QMember*)&v1_14);
    slot_tbl[15].ptr = *((QMember*)&v1_15);
    slot_tbl[16].ptr = *((QMember*)&v1_16);
    slot_tbl[17].ptr = *((QMember*)&v1_17);
    metaObj = QMetaObject::new_metaobject(
	"QEWaveForm", "QWidget",
	slot_tbl, 18,
	0, 0,
	0, 0,
	0, 0,
	0, 0 );
    return metaObj;
}
