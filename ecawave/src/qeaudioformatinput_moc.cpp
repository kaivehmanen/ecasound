/****************************************************************************
** QEAudioFormatInput meta object code from reading C++ file 'qeaudioformatinput.h'
**
** Created: Sat Feb 19 00:55:34 2000
**      by: The Qt MOC ($Id: qeaudioformatinput_moc.cpp,v 1.2 2000-02-21 23:21:48 kaiv Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEAudioFormatInput
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 7
#elif Q_MOC_OUTPUT_REVISION != 7
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

void QEAudioFormatInput::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEAudioFormatInput","QWidget");
    (void) staticMetaObject();
}

QString QEAudioFormatInput::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEAudioFormatInput",s);
}

QMetaObject* QEAudioFormatInput::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QWidget::staticMetaObject();
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
	0, 0,
	0, 0,
	0, 0,
	0, 0 );
    return metaObj;
}
