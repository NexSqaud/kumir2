#ifndef DOCBOOKVIEW_H
#define DOCBOOKVIEW_H

// Self includes
#include "document.h"

// Extension system includes
#include "extensionsystem/settings.h"

// Qt includes
#include <QString>
#include <QWidget>
#include <QUrl>
#include <QSettings>

#ifdef DOCBOOKVIEWER_LIBRARY
#define DOCBOOKVIEWER_EXPORT Q_DECL_EXPORT
#else
#define DOCBOOKVIEWER_EXPORT Q_DECL_IMPORT
#endif

namespace DocBookViewer {

class DOCBOOKVIEWER_EXPORT DocBookView
        : public QWidget
{
    Q_OBJECT
public:
    enum DocBookViewAction {
        NoAction,
        ToggleNavigationPane,
        ShowPrintDialog
    };

    explicit DocBookView(QWidget * parent = 0);
    QAction * viewerAction(const DocBookViewAction type) const;

    QStringList booksList() const;
    void activateBookIndex(int index);

    void updateSettings(ExtensionSystem::SettingsPtr settings, const QString & prefix);
    bool hasAlgorithm(const QString & name) const;
    void selectAlgorithm(const QString & name);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    ~DocBookView();

    Document addDocument(
            const QUrl & url,
            QString * error = 0,
            int index = -1
            );

    void removeDocument(const Document & existingDocument);

protected:
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *);
    void resizeEvent(QResizeEvent *);

private:    
    class DocBookViewImpl* pImpl_;
};

}

#endif
