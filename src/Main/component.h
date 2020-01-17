#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <TopoDS_Shape.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <Bnd_Box.hxx>
#include <gp_Pln.hxx>
#include <TopoDS_Edge.hxx>
struct SliceData
{
    gp_Pln plane;
    float thickness;
    TopoDS_Shape edges;
    QVector<QList<TopoDS_Edge>>layer;
    SliceData(TopoDS_Shape _edges,QVector<QList<TopoDS_Edge>> _layer,gp_Pln _plane,float _t)
    {
        edges = _edges;
        layer=_layer;
        plane=_plane;
        thickness=_t;
    }
};
class Component : public QObject
{
    Q_OBJECT
public:
    Component();
    Component(QString name,TopoDS_Shape shape);

    TopoDS_Shape getShape();
    QVector<SliceData>& getSlice();
    QString getName();
    void setBox(Bnd_Box box);
    void setSlice(QVector<SliceData>);
    Bnd_Box getBox();
private:
    void setBox();
private:
    QVector<SliceData> mySlice;
    TopoDS_Shape myShape;
    QString name;
    Bnd_Box myBox;
};

#endif // PROJECT_H
