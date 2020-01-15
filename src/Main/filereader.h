#ifndef FILEREADER_H
#define FILEREADER_H
#include <QString>
#include <QVector3D>
#include <QHash>
class FileReader
{
public:
    FileReader();
    bool ReadStlFile(const QString filename);
    uint numberTriangles;
    uint numberVertices;

    QVector<QVector3D> getnormalList();
    QVector<QVector3D> getvertices();
    QVector<QVector3D> getindices();
private:
    QVector<QVector3D> normalList;
    QVector<QVector3D> vertices;
    QVector<QVector3D> indices;
    QHash<QString,uint> verticesmap;
    QString filetype="";
    float surroundBox[6];
    float center_x=0,center_y=0;
private:
    void parseHeader(const QString path);
    bool ReadASCII(const char *buf);
    bool ReadBinary(char *buf);
    uint addPoint(QString key,QVector3D point);
};

#endif // FILEREADER_H
