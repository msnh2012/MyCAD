#include "filereader.h"
#include <QFile>
#include <QDebug>
#include <iostream>
#include <string>
#include <sstream>
#include <float.h>
#include <limits.h>
FileReader::FileReader()
{

}

QVector<QVector3D> FileReader::getnormalList()
{
    return normalList;
}

QVector<QVector3D> FileReader::getvertices()
{
    return vertices;
}

QVector<QVector3D> FileReader::getindices()
{
    return indices;
}

bool FileReader::ReadStlFile(const QString filename)
{
    normalList.clear();//清空vector
    vertices.clear();
    indices.clear();
    parseHeader(filename);

    QFile file(filename);
    uchar* buffer;
    if(file.open(QIODevice::ReadOnly))
    {
        if(filetype=="ASCII")
        {
            //qDebug()<<"File is ASCII"<<endl;
            buffer=file.map(0,file.size());
            QTextStream in(&file);
            uint total=1;
            QString line = in.readLine();
            while (!line.isNull()) {
                line = in.readLine();
                total++;
            }
            numberTriangles=(total-2)/7;
            if(buffer)
            {
                if(!ReadASCII((char*)buffer))
                {
                    return false;
                }
            }
            else
            {
                qDebug()<<"out of memory error";
                return false;
            }
        }
        else
        {
            //qDebug()<<"File is Binary"<<endl;
            buffer=file.map(0,file.size());
            if(buffer)
            {
                if(!ReadBinary((char*)buffer))
                {
                    return false;
                }
            }
            else
            {
                qDebug()<<"out of memory error"<<endl;
                return false;
            }
        }
        file.unmap(buffer);
        file.close();
        verticesmap.clear();
        return true;
    }
    else
    {
        qDebug()<<"cann't open the file:"<<filename<<endl;
        return false;
    }
}

bool FileReader::ReadBinary(char *buffer)
{
    float x=0,y=0,z=0;
    uint index=0;
    QVector3D normal,p0,p1,p2,ab,bc,nor;
    QString strx(" "),stry(" "),strz(" "),key(" ");
    char name[80];
    uint *point=new uint[3]();
    memcpy(name, buffer, 80);//80字节文件头
    //cout<<name<<endl;
    buffer += 80;
    numberTriangles=0;
    numberVertices=0;
    memcpy(&numberTriangles,buffer,4);//4字节三角面片个数
    normalList.reserve(numberTriangles);
    verticesmap.reserve(numberTriangles/2);
    //cout<<numberTriangles<<endl;
    buffer +=4;
    //读取三角形面片
    for (uint i = 0; i <numberTriangles; i++)
    {
//        cout<<i<<endl;
        memcpy(&x,buffer, 4);buffer +=4;
        memcpy(&y,buffer, 4);buffer +=4;
        memcpy(&z,buffer, 4);buffer +=4;
        //cout<<x<<" "<<y<<" "<<z<<endl;
        //normalList.push_back(Point(x, y, z));//法向量
        for (int j = 0; j < 3; j++)//读取三顶点
        {
            memcpy(&x,buffer, 4);buffer +=4;
            memcpy(&y,buffer, 4);buffer +=4;
            memcpy(&z,buffer, 4);buffer +=4;
            if(qAbs(x)<1e-12f)x=0;
            if(qAbs(y)<1e-12f)y=0;
            if(qAbs(z)<1e-12f)z=0;
            //cout<<x<<" "<<y<<" "<<z<<endl;
            strx = QString::number(double(x), 10, 15);
            stry = QString::number(double(y), 10, 15);
            strz = QString::number(double(z), 10, 15);
            //qDebug()<<strx<<" "<<stry<<" "<<strz;
            key="1"+strx+stry+strz;
            index=addPoint(key,QVector3D(x,y,z));
            point[j]=index;
        }
        indices.push_back(QVector3D(point[0],point[1],point[2]));
        buffer += 2;//跳过尾部标志
    }
    delete[] point;
    return true;
}

uint FileReader::addPoint(QString key,QVector3D point){
    uint index;
    auto it = verticesmap.find(key);
    if(it != verticesmap.end())
    {
        index=it.value();
        //cout<<"索引："<<it.value()<<endl;
    }
    else
    {
        vertices.push_back(point);
        verticesmap.insert(key,numberVertices);
        index=numberVertices;
        numberVertices++;
    }
    return index;
}

bool FileReader::ReadASCII(const char *buf)
{
    const int offset=280;
    numberVertices=0;
    int cur=0;
    double x=0, y=0, z=0;
    QVector3D normal,p0,p1,p2,ab,bc,nor;
    QString strx(" "),stry(" "),strz(" "),key(" ");
    uint index=0;
    uint *point=new uint[3]();
    const char *buffer=strstr(buf,"facet normal");
    int namelength=int(buffer-buf);
    char *name=new char[namelength];
    strncpy(name,buf,sizeof(name));
    name[namelength]='\0';
    char facet[offset];
    std::string useless;
    do
    {
        strncpy(facet,buffer,sizeof(facet));
        facet[offset-1]='\0';
        //cout<<facet<<endl;
        std::stringstream ss(facet);
        ss >> useless;//facet
        ss >> useless >> x >> y >>z;//法向量
        normal=QVector3D(x,y,z);
        normalList.push_back(QVector3D(x, y, z));
        getline(ss, useless);
        getline(ss, useless);//outer loop
        for (int i = 0; i < 3; i++)
        {
            ss >> useless >> x >> y >> z;
            if(qAbs(x)<1e-12)x=0;
            if(qAbs(y)<1e-12)y=0;
            if(qAbs(z)<1e-12)z=0;
            //cout<<x<<" "<<y<<" "<<z<<endl;
            strx = QString::number(x, 10, 15);
            stry = QString::number(y, 10, 15);
            strz = QString::number(z, 10, 15);
            //qDebug()<<strx<<" "<<stry<<" "<<strz;
            key="1"+strx+stry+strz;
            index=addPoint(key,QVector3D(x,y,z));
            point[i]=index;
        }
        indices.push_back(QVector3D(point[0],point[1],point[2]));
        getline(ss, useless);//空行
        getline(ss, useless);//end loop
        getline(ss, useless);//end facet

        buffer=strstr(buffer,"normal");
        buffer=strstr(buffer,"facet normal");
        cur++;
        //cout<<"facenumber:"<<numberTriangles<<" "<<dataset.mesh.num_faces()<<endl;
    }while(buffer!=NULL);
    delete[] point;
    return true;
}

void FileReader::parseHeader(const QString path)
{
    float xmin=FLT_MAX,xmax=FLT_MIN,ymin=FLT_MAX,ymax=FLT_MIN,zmin=FLT_MAX,zmax=FLT_MIN;
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray line = file.readLine();
        std::string header(line);
        std::string firstline = std::string(file.readLine());
        std::string::size_type idx = firstline.find("facet normal");
        if(idx != std::string::npos )
        {
            filetype="ASCII";
            uchar* buffer=file.map(0,file.size());
            if(buffer)
            {
                char*buf=(char*)buffer;
                const int offset=280;
                float x=0, y=0, z=0;
                const char *buffer=strstr(buf,"facet normal");
                int namelength=int(buffer-buf);
                char *name=new char[namelength];
                strncpy(name,buf,sizeof(name));
                char facet[offset];
                std::string useless;
                do
                {
                    strncpy(facet,buffer,sizeof(facet));
                    facet[offset-1]='\0';
                    //cout<<facet<<endl;
                    std::stringstream ss(facet);
                    ss >> useless;//facet
                    ss >> useless >> x >> y >>z;//法向量
                    getline(ss, useless);
                    getline(ss, useless);//outer loop
                    for (int i = 0; i < 3; i++)
                    {
                        ss >> useless >> x >> y >> z;
                        //cout<<x<<" "<<y<<" "<<z<<endl;
                        xmin=xmin>x ? x : xmin;
                        xmax=xmax>x ? xmax : x;
                        ymin=ymin>y ? y : ymin;
                        ymax=ymax>y ? ymax : y;
                        zmin=zmin>z ? z : zmin;
                        zmax=zmax>z ? zmax : z;
                    }
                    getline(ss, useless);//空行
                    getline(ss, useless);//end loop
                    getline(ss, useless);//end facet
                    buffer=strstr(buffer,"normal");
                    buffer=strstr(buffer,"facet normal");
                    numberTriangles++;
                }while(buffer!=nullptr);
            }
            else
            {
                std::cout<<"out of memory error"<<endl;
            }
            file.unmap(buffer);
        }
        else
        {
            filetype="Binary";
            uchar* buffer=file.map(0,file.size());
            if(buffer)
            {
                float x=0,y=0,z=0.0f;
                buffer += 80;
                memcpy(&numberTriangles,buffer,4);//4字节三角面片个数
                //cout<<numberTriangles<<endl;
                buffer +=4;
                //读取三角形面片
                for (uint i = 0; i <numberTriangles; i++)
                {
                    buffer +=12;
                    for (int j = 0; j < 3; j++)//读取三顶点
                    {
                        memcpy(&x,buffer, 4);buffer +=4;
                        memcpy(&y,buffer, 4);buffer +=4;
                        memcpy(&z,buffer, 4);buffer +=4;
                        xmin=xmin>x ? x : xmin;
                        xmax=xmax>x ? xmax : x;
                        ymin=ymin>y ? y : ymin;
                        ymax=ymax>y ? ymax : y;
                        zmin=zmin>z ? z : zmin;
                        zmax=zmax>z ? zmax : z;
                        //cout<<x<<" "<<y<<" "<<z<<endl;
                    }
                    buffer += 2;//跳过尾部标志
                }
            }
            else
            {
                std::cout<<"out of memory error"<<endl;
            }
            file.unmap(buffer);
        }
        file.close();
    }
    else
    {
        qDebug()<<"cann't open the file:"<<path<<endl;
    }
    surroundBox[0]=xmin;
    surroundBox[1]=xmax;
    surroundBox[2]=ymin;
    surroundBox[3]=ymax;
    surroundBox[4]=zmin;
    surroundBox[5]=zmax;
    center_x = (xmin+xmax)/2.0f;
    center_y = (ymin+ymax)/2.0f;
}
