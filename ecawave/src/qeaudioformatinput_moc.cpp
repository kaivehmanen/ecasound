/****************************************************************************
** QEAudioFormatInput meta object code from reading C++ file 'qeaudioformatinput.h'
**
** Created: Tue Mar 7 00:35:42 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.3 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEAudioFormatInput
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qeaudioformatinput.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEAudioFormatInput::className() const
{
    return "QEAudioFormatInput";
}

QMetaObject *QEAudioFormatInput::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QEAudioFormatInput(&QEAudioFormatInput::staticMetaObject);

#endif

void QEAudioFormatInput::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEAudioFormatInput","QWidget");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QEAudioFormatInput::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEAudioFormatInput",s);
}

void QEAudioFormatInput::staticMetaObject()
{
    if ( metaObj )
	return;
    QWidget::staticMetaObject();
#else

    QWidget::initMetaObject();
#endif

    typedef void(QEAudioFormatInput::*m1_t0)(int);
    typedef void(QEAudioFormatInput::*m1_t1)(int);
    typedef void(QEAudioFormatInput::*m1_t2)(int);
    typedef void(QEAudioFormatInput::*m1_t3)();
    typedef void(QEAudioFormatInput::*m1_t4)();
    m1_t0 v1_0 = Q_AMPERSAND QEAudioFormatInput::set_bits;
    m1_t1 v1_1 = Q_AMPERSAND QEAudioFormatInput::set_channels;
    m1_t2 v1_2 = Q_AMPERSAND QEAudioFormatInput::set_srate;
    m1_t3 v1_3 = Q_AMPERSAND QEAudioFormatInput::enable_format;
    m1_t4 v1_4 = Q_AMPERSAND QEAudioFormatInput::disable_format;
    QMetaData *slot_tbl = QMetaObject::new_metadata(5);
    slot_tbl[0].name = "set_bits(int)";
    slot_tbl[1].name = "set_channels(int)";
    slot_tbl[2].name = "set_srate(int)";
    slot_tbl[3].name = "enable_format()";
    slot_tbl[4].name = "disable_format()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    metaObj = QMetaObject::new_metaobject(
	"QEAudioFormatInput", "QWidget",
	slot_tbl, 5,
	0, 0 );
}
