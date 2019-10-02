#ifndef EDITORPLANE_H
#define EDITORPLANE_H

#include <kumir2-libs/extensionsystem/settings.h>
#include <kumir2-libs/docbookviewer/docbookview.h>
#include <kumir2/analizer_instanceinterface.h>

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <kumir2/lexemtype.h>
#include <kumir2/analizerinterface.h>


namespace Editor {

class EditorPlane : public QWidget
{
    friend class Editor;
    Q_OBJECT
public:
    explicit EditorPlane(class EditorInstance * editor);

    uint widthInChars() const;
    uint charWidth() const;
    uint lineHeight() const;
    int marginCharactersCount() const;
    QRect cursorRect() const;
    uint marginLeftBound() const;
    QRect marginBackgroundRect() const;
    QRect marginLineRect() const;
    uint normalizedNewMarginLinePosition(uint x) const;
    static QString MarginWidthKey;
    static uint MarginWidthDefault;
    void addContextMenuAction(QAction * a);
    void updateScrollBars();
    void findCursor();
    void ensureCursorVisible();
    void updateAnalizer();

public slots:
    void selectAll();
    void copy();
    void paste();
    void cut();
    void removeLine();
    void removeLineTail();
    void setLineHighlighted(int lineNo, const QColor & color, quint32 colStart, quint32 colEnd);
    void updateSettings(const QStringList & keys);
protected:
    enum MouseAreaPart { NoActionPart, LockSymbolPart, BreakpointPart, TextAreaPart, MarginLinePart, MarginPart };

    static QPolygon errorUnderline(int x, int y, int len);
    void dragEventHandler(QDragMoveEvent * e);
    void wheelEvent(QWheelEvent *);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dropEvent(QDropEvent *e);
    void resizeEvent(QResizeEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    bool canDrop(const QPoint &pos, const QMimeData * data) const;
    uint leftTextAreaPosition() const;
    uint rightTextAreaPosition() const;
    uint lockSymbolWidth() const;
    MouseAreaPart partAtPosition(const QPoint & pos) const;
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    uint textLeftPosition() const;
    bool eventFilter(QObject *, QEvent *);
    void leaveEvent(QEvent *e);
    void contextMenuEvent(QContextMenuEvent *);
    void initMouseCursor();    
    void ensureHighlightedLineVisible();
//    void updateOffset();
    void paintEvent(QPaintEvent *);
    void paintLineHighlight(QPainter * painter, const QRect & rect);
    bool event(QEvent *e);
    void paintLockSymbol(QPainter * p, bool colored, const QRect & r);
    QPoint offset() const;
    void paintBackground(QPainter *p, const QRect &rect);
    void paintMarginBackground(QPainter *p, const QRect & rect);

    void paintMarginText(QPainter *p, const QRect & rect);
    void paintCursor(QPainter *p, const QRect &rect);
    void paintText(QPainter *p, const QRect &rect);
    void paintProgramStructureLines(QPainter *p, const QRect &rect);
    void paintHiddenTextDelimeterLine(QPainter *p);
    void paintSelection(QPainter *p, const QRect &rect);
    void paintRectSelection(QPainter *p, const QRect &rect);
    void paintLineNumbers(QPainter *p, const QRect &rect);
    void paintNewMarginLine(QPainter *p);
    void paintNewHiddenDelimeterLine(QPainter *p);
    void paintDropPosition(QPainter *p);
    void keyPressEvent(QKeyEvent *);
    void doAutocomplete();
    void keyReleaseEvent(QKeyEvent *);
    void setProperFormat(QPainter * p, Shared::LexemType type, const QChar &c);

    QString tryCorrectKeyboardLayout(const QString &source) const;
    void tryCorrectKeyboardLayoutForLastLexem();

protected slots:
    void updateCursor();
    void updateText(int fromLine, int toLine);
    void finishAutoCompletion(const QString & suggession);
    void ensureMarginNotTooLarge();


private:
    int timerId_;
    class EditorInstance *editor_;
    Shared::Analizer::InstanceInterface *analizer_;
    Shared::Analizer::HelperInterface *helper_;
    bool caseInsensitive_;

    QPoint marginMousePressedPoint_;
    QPoint delimeterRuleMousePressedPoint_;
    QPoint textMousePressedPoint_;
    QPoint textPressedPosition_;
    bool selectionInProgressFlag_;

    QPoint pnt_dropPosMarker;
    QPoint pnt_dropPosCorner;
    int marginBackgroundAlpha_;

    int highlightedTextLineNumber_;
    QColor highlightedTextLineColor_;
    int highlightedLockSymbolLineNumber_;
    quint32 highlightedTextColumnStartNumber_;
    quint32 highlightedTextColumnEndNumber_ ;
    QList<QAction*> contextMenuActions_;

    QLabel * marginHintBox_;
    bool escPressFlag_;
    bool typeTextFlag_;

signals:
    void urlsDragAndDropped(const QList<QUrl> &);
    void requestAutoScroll(char a);
    void requestAutoScrollX(char a);
    void disableInsertActions();
    void enableInsertActions();
    void message(const QString &text);

    void breakpointCnagedOrInserted(bool enabled, quint32 lineNo, quint32 ignoreCount, const QString & condition);
    void breakpointRemoved(quint32 lineNo);


};

} // namespace Editor

#endif // EDITORPLANE_H
