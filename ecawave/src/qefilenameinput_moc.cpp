/****************************************************************************
** QEFilenameInput meta object code from reading C++ file 'qefilenameinput.h'
**
** Created: Thu Jan 27 01:53:00 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.1 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEFilenameInput
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qefilenameinput.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEFilenameInput::className() const
{
    return "QEFilenameInput";
}

QMetaObject *QEFilenameInput::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QEFilenameInput(&QEFilenameInput::staticMetaObject);

#endif

void QEFilenameInput::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("QEFilenameInput","QWidget");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QEFilenameInput::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEFilenameInput",s);
}

void QEFilenameInput::staticMetaObject()
{
    if ( metaObj )
	return;
    QWidget::staticMetaObject();
#else

    QWidget::initMetaObject();
#endif

    typedef void(QEFilenameInput::*m1_t0)();
    m1_t0 v1_0 = Q_AMPERSAND QEFilenameInput::button_browse;
    QMetaData *slot_tbl = QMetaObject::new_metadata(1);
    slot_tbl[0].name = "button_browse()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(QEFilenameInput::*m2_t0)();
    m2_t0 v2_0 = Q_AMPERSAND QEFilenameInput::file_selected;
    QMetaData *signal_tbl = QMetaObject::new_metadata(1);
    signal_tbl[0].name = "file_selected()";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = QMetaObject::new_metaobject(
	"QEFilenameInput", "QWidget",
	slot_tbl, 1,
	signal_tbl, 1 );
}

// SIGNAL file_selected
void QEFilenameInput::file_selected()
{
    activate_signal( "file_selected()" );
}
