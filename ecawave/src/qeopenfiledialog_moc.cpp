/****************************************************************************
** QEOpenFileDialog meta object code from reading C++ file 'qeopenfiledialog.h'
**
** Created: Sun Jan 23 03:51:01 2000
**      by: The Qt Meta Object Compiler ($Revision: 1.1 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_QEOpenFileDialog
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./qeopenfiledialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *QEOpenFileDialog::className() const
{
    return "QEOpenFileDialog";
}

QMetaObject *QEOpenFileDialog::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QEOpenFileDialog(&QEOpenFileDialog::staticMetaObject);

#endif

void QEOpenFileDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("QEOpenFileDialog","QDialog");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QEOpenFileDialog::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("QEOpenFileDialog",s);
}

void QEOpenFileDialog::staticMetaObject()
{
    if ( metaObj )
	return;
    QDialog::staticMetaObject();
#else

    QDialog::initMetaObject();
#endif

    typedef void(QEOpenFileDialog::*m1_t0)();
    typedef void(QEOpenFileDialog::*m1_t1)(bool);
    typedef void(QEOpenFileDialog::*m1_t2)(bool);
    m1_t0 v1_0 = Q_AMPERSAND QEOpenFileDialog::format_test;
    m1_t1 v1_1 = Q_AMPERSAND QEOpenFileDialog::update_refresh_toggle;
    m1_t2 v1_2 = Q_AMPERSAND QEOpenFileDialog::update_wcache_toggle;
    QMetaData *slot_tbl = QMetaObject::new_metadata(3);
    slot_tbl[0].name = "format_test()";
    slot_tbl[1].name = "update_refresh_toggle(bool)";
    slot_tbl[2].name = "update_wcache_toggle(bool)";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    metaObj = QMetaObject::new_metaobject(
	"QEOpenFileDialog", "QDialog",
	slot_tbl, 3,
	0, 0 );
}
