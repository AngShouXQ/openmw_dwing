#ifndef CSV_WIDGET_SCENETOOLSHAPEBRUSH_H
#define CSV_WIDGET_SCENETOOLSHAPEBRUSH_H

#include <QFrame>
#include <QModelIndex>

#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>

#ifndef Q_MOC_RUN
#include "brushshapes.hpp"
#include "scenetool.hpp"

#include "../../model/doc/document.hpp"
#endif

class QTableWidget;

namespace CSVRender
{
    class TerrainShapeMode;
}

namespace CSVWidget
{
    /// \brief Layout-box for some brush button settings
    class ShapeBrushSizeControls : public QGroupBox
    {
        Q_OBJECT

    public:
        ShapeBrushSizeControls(const QString& title, QWidget* parent);

    private:
        QSlider* mBrushSizeSlider = new QSlider(Qt::Horizontal);
        QSpinBox* mBrushSizeSpinBox = new QSpinBox;

        friend class SceneToolShapeBrush;
        friend class CSVRender::TerrainShapeMode;
    };

    /// \brief Brush settings window
    class ShapeBrushWindow : public QFrame
    {
        Q_OBJECT

    public:
        ShapeBrushWindow(CSMDoc::Document& document, QWidget* parent = nullptr);
        void configureButtonInitialSettings(QPushButton* button);

        const QString toolTipPoint = "Paint single point";
        const QString toolTipSquare = "Paint with square brush";
        const QString toolTipCircle = "Paint with circle brush";
        const QString toolTipCustom = "Paint with custom brush, defined by terrain selection";

    private:
        CSVWidget::BrushShape mBrushShape = CSVWidget::BrushShape_Point;
        int mBrushSize = 1;
        CSMDoc::Document& mDocument;
        QGroupBox* mHorizontalGroupBox;
        QComboBox* mToolSelector;
        QSlider* mToolStrengthSlider;
        QPushButton* mButtonPoint;
        QPushButton* mButtonSquare;
        QPushButton* mButtonCircle;
        QPushButton* mButtonCustom;
        ShapeBrushSizeControls* mSizeSliders;

        friend class SceneToolShapeBrush;
        friend class CSVRender::TerrainShapeMode;

    public slots:
        void setBrushShape();
        void setBrushSize(int brushSize);

    signals:
        void passBrushSize(int brushSize);
        void passBrushShape(CSVWidget::BrushShape brushShape);
    };

    class SceneToolShapeBrush : public SceneTool
    {
        Q_OBJECT

        QString mToolTip;
        CSMDoc::Document& mDocument;
        QFrame* mPanel;
        QTableWidget* mTable;
        ShapeBrushWindow* mShapeBrushWindow;

    private:
        void adjustToolTips();

    public:
        SceneToolShapeBrush(SceneToolbar* parent, const QString& toolTip, CSMDoc::Document& document);

        void showPanel(const QPoint& position) override;
        void updatePanel();

        void dropEvent(QDropEvent* event) override;
        void dragEnterEvent(QDragEnterEvent* event) override;

        friend class CSVRender::TerrainShapeMode;

    public slots:
        void setButtonIcon(CSVWidget::BrushShape brushShape);
        void clicked(const QModelIndex& index);
        void activate() override;

    signals:
        void passEvent(QDropEvent* event);
        void passEvent(QDragEnterEvent* event);
    };
}

#endif
