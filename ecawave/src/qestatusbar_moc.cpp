/****************************************************************************
** QEStatusBar meta object code from reading C++ file 'qestatusbar.h'
**
** Created: Tue Mar 7 00:36:00 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.3 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEStatusBar
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qestatusbar.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEStatusBar::className() const
{
    return "QEStatusBar";
}

QMetaObject *QEStatusBar::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QEStatusBar(&QEStatusBar::staticMetaObject);

#endif

void QEStatusBar::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QStatusBar::className(), "QStatusBar") != 0 )
	badSuperclassWarning("QEStatusBar","QStatusBar");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QEStatusBar::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEStatusBar",s);
}

void QEStatusBar::staticMetaObject()
{
    if ( metaObj )
	return;
    QStatusBar::staticMetaObject();
#else

    QStatusBar::initMetaObject();
#endif

    typedef void(QEStatusBar::*m1_t0)(ECA_AUDIO_TIME);
    typedef void(QEStatusBar::*m1_t1)(ECA_AUDIO_TIME,ECA_AUDIO_TIME);
    typedef void(QEStatusBar::*m1_t2)(ECA_AUDIO_TIME,ECA_AUDIO_TIME);
    typedef void(QEStatusBar::*m1_t3)(bool);
    typedef void(QEStatusBar::*m1_t4)();
    m1_t0 v1_0 = Q_AMPERSAND QEStatusBar::current_position;
    m1_t1 v1_1 = Q_AMPERSAND QEStatusBar::visible_area;
    m1_t2 v1_2 = Q_AMPERSAND QEStatusBar::marked_area;
    m1_t3 v1_3 = Q_AMPERSAND QEStatusBar::toggle_editing;
    m1_t4 v1_4 = Q_AMPERSAND QEStatusBar::update;
    QMetaData *slot_tbl = QMetaObject::new_metadata(5);
    slot_tbl[0].name = "current_position(ECA_AUDIO_TIME)";
    slot_tbl[1].name = "visible_area(ECA_AUDIO_TIME,ECA_AUDIO_TIME)";
    slot_tbl[2].name = "marked_area(ECA_AUDIO_TIME,ECA_AUDIO_TIME)";
    slot_tbl[3].name = "toggle_editing(bool)";
    slot_tbl[4].name = "update()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    metaObj = QMetaObject::new_metaobject(
	"QEStatusBar", "QStatusBar",
	slot_tbl, 5,
	0, 0 );
}
