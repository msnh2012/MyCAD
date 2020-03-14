#ifndef SLICE_H
#define SLICE_H

#include <QDialog>
#include "DocumentCommon.h"
#include <QVector3D>
#include <TopoDS_Face.hxx>
namespace Ui {
class Slice;
}
class LoopData
{
public:
    LoopData(QPolygonF polygon,bool isfill=true);
public:
    bool IsClockWise();
    double getArea();
    QPolygonF getPolygon();
    bool IsSelfIntersection();
    bool smooth();
private:
    QPolygonF _polygon;
    bool _isfill = true;
    bool CheckSelfIntersection();
    double ComputePolygonArea();
};

class Contour
{
public:
    double minZLevel =0.0;
    unsigned number_boundaries =0;
    std::vector<LoopData> contour;
    Contour()
    {

    }
    void clear()
    {
        contour.clear();
    }

    double getZ() const
    {
        return minZLevel;
    }
    bool IsIsSelfIntersection();
};

class Slice : public QDialog
{
    Q_OBJECT

public:
    enum SliceDirection{Slice_X=0,Slice_Y=1,Slice_Z=2};
    explicit Slice(QWidget *parent = nullptr);
    ~Slice();
    void setData(Handle(TopTools_HSequenceOfShape) aSequence,DocumentCommon *doc,SliceDirection sd=Slice_Z);
private slots:
    void on_pushButton_clicked(bool checked);

    void on_checkBox_stateChanged(int arg1);

    void on_verticalSlider_valueChanged(int value);

    void on_comboBox_currentIndexChanged(int index);

    void on_exportBtn_clicked();

    void updateProgressBar(int value);
signals:
    void sglStep(int );
private:
    void getPlaneSet(const Bnd_Box &box);
    QVector<QList<TopoDS_Edge>> splitEdges(TopoDS_Face &aFace);
    QVector<QPointF> discretizationOfEdge(const TopoDS_Edge &edge,float abscissa)const;
    QString gp_PntToQString(gp_Pnt&);

    bool exportSlice(const QString fileName, const QVector<Contour> &result);
    bool ExportSLC(const QString fileName,const QVector<Contour> &result);
    bool ExportCLIBINARY(const QString fileName, const QVector<Contour> &result);
    bool ExportCLIASCII(const QString fileName,const QVector<Contour> &result);
    bool ChooseFilePath(QString &filename);

private:
    Ui::Slice *ui;
    Handle(TopTools_HSequenceOfShape) aSequence;
    DocumentCommon *doc;
    double thickness = 0;
    gp_Dir normal;//slice direction
    QVector<QVector3D> planeset;
    SliceDirection sd;
    QVector<int> sliceindex;

    Handle(AIS_InteractiveContext) myContext;
    View *openGLWidget;
    Handle(V3d_Viewer) aViewer;

    double theXmin,theYmin,theZmin,theXmax,theYmax,theZmax;
    double width,length;
};

#endif // SLICE_H
