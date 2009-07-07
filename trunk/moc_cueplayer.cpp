/****************************************************************************
** Meta object code from reading C++ file 'cueplayer.h'
**
** Created: Sun Jul 5 21:22:43 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "cueplayer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cueplayer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CuePlayer[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x08,
      27,   10,   10,   10, 0x08,
      56,   10,   10,   10, 0x08,
      72,   10,   10,   10, 0x08,
      88,   10,   10,   10, 0x08,
     100,   10,   10,   10, 0x08,
     112,   10,   10,   10, 0x08,
     127,  125,   10,   10, 0x08,
     169,   10,   10,   10, 0x08,
     182,   10,   10,   10, 0x08,
     198,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CuePlayer[] = {
    "CuePlayer\0\0setNumLCDs(int)\0"
    "cueFileSelected(QStringList)\0"
    "playNextTrack()\0playPrewTrack()\0"
    "playTrack()\0stopTrack()\0pauseTrack()\0"
    ",\0stateChanged(Phonon::State,Phonon::State)\0"
    "tick(qint64)\0sliderRelease()\0"
    "volumeValue(int)\0"
};

const QMetaObject CuePlayer::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CuePlayer,
      qt_meta_data_CuePlayer, 0 }
};

const QMetaObject *CuePlayer::metaObject() const
{
    return &staticMetaObject;
}

void *CuePlayer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CuePlayer))
        return static_cast<void*>(const_cast< CuePlayer*>(this));
    if (!strcmp(_clname, "Ui::CuePlayer"))
        return static_cast< Ui::CuePlayer*>(const_cast< CuePlayer*>(this));
    return QWidget::qt_metacast(_clname);
}

int CuePlayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setNumLCDs((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: cueFileSelected((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 2: playNextTrack(); break;
        case 3: playPrewTrack(); break;
        case 4: playTrack(); break;
        case 5: stopTrack(); break;
        case 6: pauseTrack(); break;
        case 7: stateChanged((*reinterpret_cast< Phonon::State(*)>(_a[1])),(*reinterpret_cast< Phonon::State(*)>(_a[2]))); break;
        case 8: tick((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 9: sliderRelease(); break;
        case 10: volumeValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        }
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
