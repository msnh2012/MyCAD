#include "slice.h"
#include "ui_slice.h"
#include <QDebug>
#include "ApplicationCommon.h"
#include "BRepAlgoAPI_Section.hxx"
#include "OpenGl_GraphicDriver.hxx"
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Curve3D.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <GeomAPI.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom_Line.hxx>
#include "View.h"
#include <QElapsedTimer>
#include <QPointF>
#include <QPolygonF>
#include <QFile>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
Slice::Slice(QWidget *parent):QDialog(parent),ui(new Ui::Slice)
{
    ui->setupUi(this);

    static Handle(OpenGl_GraphicDriver) aGraphicDriver;

    if (aGraphicDriver.IsNull())
    {
      Handle(Aspect_DisplayConnection) aDisplayConnection;
  #if !defined(_WIN32) && !defined(__WIN32__) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX))
      aDisplayConnection = new Aspect_DisplayConnection (OSD_Environment ("DISPLAY").Value());
  #endif
      aGraphicDriver = new OpenGl_GraphicDriver (aDisplayConnection);
    }

    aViewer = new V3d_Viewer (aGraphicDriver);
    aViewer->SetDefaultViewSize (1000.0);
    aViewer->SetDefaultViewProj (V3d_XposYnegZpos);
    aViewer->SetComputedMode (Standard_True);
    aViewer->SetDefaultComputedMode (Standard_True);
    aViewer->SetDefaultLights();
    aViewer->SetLightOn();
    myContext = new AIS_InteractiveContext (aViewer);
    myContext->SetDisplayMode( 1, false);
    openGLWidget = new View(myContext,this);
    openGLWidget->setObjectName(QString::fromUtf8("openGLWidget"));

    ui->horizontalLayout_3->addWidget(openGLWidget);
    ui->progressBar->setVisible(false);
    normal = gp_Dir(0,0,1);
    sd = Slice_Z;
    //ui->isParallel->setVisible(false);
    connect(this,SIGNAL(sglStep(int)),this,SLOT(updateProgressBar(int)));
}

Slice::~Slice()
{
    delete ui;
    delete openGLWidget;
}

void Slice::setData(Handle(TopTools_HSequenceOfShape) Sequence,DocumentCommon *adoc,SliceDirection sd)
{
    aSequence=Sequence;
    doc = adoc;
    ui->comboBox->clear();
    sliceindex.clear();
    for ( int i = 1; i <= aSequence->Length(); i++ )
    {
        TopoDS_Shape shape = aSequence->Value(i);
        Component * cur;
        for (int j=0;j<doc->myComponents.size();j++) {
            if(doc->myComponents[j]->getShape()==shape)
            {
                cur = doc->myComponents[j];
                sliceindex.append(j);
            }
        }
        ui->comboBox->addItem(cur->getName(),i-1);
    }
    if(sd != Slice_Z)
    {
        if(sd == Slice_X)
        {
            sd = Slice_X;
            normal = gp_Dir(1,0,0);
        }
        else{
            sd = Slice_Y;
            normal = gp_Dir(0,1,0);
        }
    }
}

void Slice::on_pushButton_clicked(bool checked)
{
    QElapsedTimer time;
    time.start();
    ui->progressBar->setVisible(true);
    thickness = ui->thickSpinBox->value();

    float interval = 100.0f / aSequence->Length();
    for ( int i = 1; i <= aSequence->Length(); i++ )
    {
        TopoDS_Shape shape = aSequence->Value(i);
        Component * cur;
        for (int j=0;j<doc->myComponents.size();j++) {
            if(doc->myComponents[j]->getShape()==shape)
            {
                cur = doc->myComponents[j];
            }
        }

        QVector<SliceData> slicedata;
        getPlaneSet(cur->getBox());
        int numberlayer=planeset.size();

        gp_XYZ p1,p2,p3,p4;
        TopoDS_Vertex V1,V2,V3,V4;
        TopoDS_Wire RectWire;
        TopoDS_Face RectFace;

        if(ui->isParallel->isChecked())
        {
            #pragma omp parallel for num_threads(8)
            for (int l=0;l<numberlayer;l++) {
                gp_Pln plane(gp_Pnt(planeset[l].x(),planeset[l].y(),planeset[l].z()),normal);
                BRepAlgoAPI_Section S(shape,plane,Standard_False);
                S.Approximation(Standard_True);
                S.Build();
                S.SetRunParallel(true);
                QVector<QList<TopoDS_Edge>> layer;
                slicedata.append(SliceData(S.Shape(),layer,plane,thickness));
                int fraction = interval*(i-1+float(l)/numberlayer);
                //emit sglStep(fraction);
            }
        }
        else{
            for (int l=0;l<numberlayer;l++) {
                gp_Pln plane(gp_Pnt(planeset[l].x(),planeset[l].y(),planeset[l].z()),normal);
                p1.SetCoord(theXmin-width/2,theYmin-length/2,planeset[l].z() + 0.001);
                p2.SetCoord(theXmax+width/2,theYmin-length/2,planeset[l].z() + 0.001);
                p3.SetCoord(theXmax+width/2,theYmax+length/2,planeset[l].z()+ 0.001);
                p4.SetCoord(theXmin-width/2,theYmax+length/2,planeset[l].z() + 0.001);
                V1 = BRepBuilderAPI_MakeVertex(p1);
                V2 = BRepBuilderAPI_MakeVertex(p2);
                V3 = BRepBuilderAPI_MakeVertex(p3);
                V4 = BRepBuilderAPI_MakeVertex(p4);
                RectWire = BRepBuilderAPI_MakePolygon(V1,V2,V3,V4,Standard_True);
                RectFace = BRepBuilderAPI_MakeFace(RectWire);

                TopoDS_Shape aCommon = BRepAlgoAPI_Common(shape,RectFace);
                TopoDS_Face aFace;
                for(TopExp_Explorer ex(aCommon,TopAbs_FACE);ex.More();ex.Next())
                {
                    aFace = TopoDS::Face(ex.Current());
                }
                QVector<QList<TopoDS_Edge>> layer=splitEdges(aFace);
                slicedata.append(SliceData(aCommon,layer,plane,thickness));
                int fraction = interval*(i-1+float(l)/numberlayer);
                emit sglStep(fraction);
            }
        }
        cur->setSlice(slicedata);
    }
    ui->progressBar->setValue(100);
    int j = ui->comboBox->currentIndex();
    QVector<SliceData> slicedata =doc->myComponents[j]->getSlice();
    ui->layerCount->setText("1/"+QString::number(slicedata.size()));
    ui->verticalSlider->setRange(1,slicedata.size());
    on_comboBox_currentIndexChanged(ui->comboBox->currentIndex());
    float slicetime = time.elapsed()/1000.0f;
    doc->getApplication()->BEdt->append("slice time :"+QString::number(slicetime));
    ui->progressBar->setVisible(false);
}

void Slice::getPlaneSet(const Bnd_Box &box)
{
    planeset.clear();
    double min,max;
    box.Get(theXmin,theYmin,theZmin,theXmax,theYmax,theZmax);
    width = theXmax-theXmin;
    length = theYmax- theYmin;
    //qDebug()<<theXmin<<" "<<theXmax<<" "<<theYmin<<" "<<theYmax<<" "<<theZmin<<" "<<theZmax;
    float initial_layer_thickness;
    int layernumber;
    switch (sd) {
    case Slice_X:
        min = theXmin;
        max = theXmax;
        initial_layer_thickness=min+thickness/2.0f;
        layernumber=int((max-initial_layer_thickness)/thickness)+1;
        planeset.resize(layernumber);
        planeset[0] = QVector3D(initial_layer_thickness,0,0);
        for (int i=1;i<layernumber;i++) {
            planeset[i]=QVector3D(initial_layer_thickness+thickness*i,0,0);
        }
        break;
    case Slice_Y:
        min = theYmin;
        max = theYmax;
        initial_layer_thickness=min+thickness/2.0f;
        layernumber=int((max-initial_layer_thickness)/thickness)+1;
        planeset.resize(layernumber);
        planeset[0] = QVector3D(0,initial_layer_thickness,0);
        for (int i=1;i<layernumber;i++) {
            planeset[i]=QVector3D(0,initial_layer_thickness+thickness*i,0);
        }
        break;
    case Slice_Z:
        min = theZmin;
        max = theZmax;
        initial_layer_thickness=min+thickness/2.0f;
        layernumber=int((max-initial_layer_thickness)/thickness)+1;
        planeset.resize(layernumber);
        planeset[0] = QVector3D(0,0,initial_layer_thickness);
        for (int i=1;i<layernumber;i++) {
            planeset[i]=QVector3D(0,0,initial_layer_thickness+thickness*i);
        }
        break;
    default:
        break;
    }

}

void Slice::on_checkBox_stateChanged(int arg1)
{
    if(sliceindex.empty())return;
    if(arg1 == 0)
    {
        myContext->EraseAll(Standard_True);
        int j = ui->comboBox->currentIndex();
        QVector<SliceData> slicedata =doc->myComponents[j]->getSlice();
        ui->layerCount->setText("1/"+QString::number(slicedata.size()));
        myContext->Display( new AIS_Shape( slicedata.at( 1 ).edges ), false );
        myContext->UpdateCurrentViewer();
    }
    else{
        myContext->EraseAll(Standard_True);
        int j = ui->comboBox->currentIndex();
        QVector<SliceData> slicedata =doc->myComponents[j]->getSlice();
        for ( int i = 0; i < slicedata.size(); i++ )
        {
          myContext->Display( new AIS_Shape( slicedata.at( i ).edges ), false );
        }
        myContext->UpdateCurrentViewer();
    }
}

void Slice::on_verticalSlider_valueChanged(int value)
{
    if(ui->checkBox->isChecked()) return;
    myContext->EraseAll(Standard_True);
    int j = ui->comboBox->currentIndex();
    QVector<SliceData> slicedata =doc->myComponents[j]->getSlice();
    ui->layerCount->setText(QString::number(value)+"/"+QString::number(slicedata.size()));
    myContext->Display( new AIS_Shape( slicedata.at(value-1).edges ), false );
    myContext->UpdateCurrentViewer();
}

void Slice::on_comboBox_currentIndexChanged(int index)
{
    if(sliceindex.empty())return;
    if(!ui->checkBox->isChecked())
    {
        myContext->EraseAll(Standard_True);
        QVector<SliceData> slicedata =doc->myComponents[index]->getSlice();
        ui->layerCount->setText("1/"+QString::number(slicedata.size()));
        myContext->Display( new AIS_Shape( slicedata.at(0).edges ), false );
        myContext->UpdateCurrentViewer();
    }
    else{
        myContext->EraseAll(Standard_True);
        QVector<SliceData> slicedata =doc->myComponents[index]->getSlice();
        for ( int i = 0; i < slicedata.size(); i++ )
        {
          myContext->Display( new AIS_Shape( slicedata.at(i).edges), false );
        }
        myContext->UpdateCurrentViewer();
    }
}

void Slice::on_exportBtn_clicked()
{
    qDebug()<<"export button is clicked.";

    QString filename;
    bool success = ChooseFilePath(filename);
    if(!success){
        QMessageBox::information(this,tr("Failed"),tr("Failed to choose the path!"));
    }
    int j = ui->comboBox->currentIndex();
    QVector<SliceData> slicedata =doc->myComponents[j]->getSlice();
    QVector<Contour> result;
    for ( int i = 0; i < slicedata.size(); i++ )
    {
        QVector<QList<TopoDS_Edge>> layer = slicedata[i].layer;
        Contour contour;
        contour.minZLevel = planeset[i].z();
        contour.number_boundaries = layer.size();
        for (int i=0;i<layer.size();i++) {
            //qDebug()<<"contour "<<i<<" :";
            QList<TopoDS_Edge> con = layer[i];
            QVector<QPointF> points;
            for (TopoDS_Edge edge : con) {
                QVector<QPointF> discretizatededge = discretizationOfEdge(edge,0.5);
                if(points.empty())
                {
                    points.append(discretizatededge);
                }
                else{
                    QPointF head = points.front();
                    QPointF tail = points.back();
                    //qDebug()<<head<<" "<<tail;
                    QPointF start = discretizatededge.front();
                    QPointF end = discretizatededge.back();
                    //qDebug()<<start<<" "<<end<<endl;

                    qreal err = 1e-6;
                    if((head-start).manhattanLength()<err)
                    {
                        discretizatededge.pop_front();
                        for(QPointF p:discretizatededge)
                        {
                            points.push_front(p);
                        }
                    }
                    else if((head-end).manhattanLength()<err)
                    {
                        discretizatededge.pop_back();
                        for(auto it =discretizatededge.rbegin();it!=discretizatededge.rend();it++)
                        {
                            points.push_front(*it);
                        }
                    }
                    if((tail-start).manhattanLength()<err)
                    {
                        discretizatededge.pop_front();
                        points.append(discretizatededge);
                    }
                    if((tail-end).manhattanLength()<err)
                    {
                        discretizatededge.pop_back();
                        for(auto it =discretizatededge.rbegin();it!=discretizatededge.rend();it++)
                        {
                            points.push_back(*it);
                        }
                    }
                }
            }

            QPolygonF polygon(points);
            contour.contour.push_back(LoopData(polygon));
            //qDebug()<<polygon;
        }
        result.push_back(contour);
    }

    success=exportSlice(filename,result);
    if(!success){
        QMessageBox::critical(this,tr("Failed"),tr("Failed to export slice!"));
    }
    else{
        QMessageBox::information(this,tr("Success"),"Save to "+ filename);
    }
}

QVector<QList<TopoDS_Edge>> Slice::splitEdges(TopoDS_Face &aFace)
{
    QVector<QList<TopoDS_Edge>> res;

    TopoDS_Wire OuterWire =BRepTools::OuterWire(aFace);
    QList<TopoDS_Edge> list;
    for(BRepTools_WireExplorer ex(OuterWire);ex.More();ex.Next())
    {
        TopoDS_Edge edge = TopoDS::Edge(ex.Current());
        if(edge.Orientation() == TopAbs_REVERSED )edge.Orientation(TopAbs_FORWARD);
        list.push_back(edge);
//        gp_Pnt first,last;
//        getEndpointOfEdge(edge,first,last);
//        qDebug()<<edge.Orientation()<<" "<<first.X()<<" "<<first.Y()<<" "<<first.Z()<<" "<<last.X()<<" "<<last.Y()<<" "<<last.Z();
    }
    res.push_back(list);
    for(TopExp_Explorer ex(aFace,TopAbs_WIRE);ex.More();ex.Next())
    {
        TopoDS_Wire Wire = TopoDS::Wire(ex.Current());
        if(Wire.IsSame(OuterWire))continue;
        //qDebug()<<"Wire "<<res.size()<<": ";
        list.clear();
        for(BRepTools_WireExplorer ex(Wire);ex.More();ex.Next())
        {
            TopoDS_Edge edge = TopoDS::Edge(ex.Current());
            if(edge.Orientation() == TopAbs_FORWARD  )edge.Reverse();
            list.push_back(edge);
//            gp_Pnt first,last;
//            getEndpointOfEdge(edge,first,last);
//            qDebug()<<edge.Orientation()<<" "<<first.X()<<" "<<first.Y()<<" "<<first.Z()<<" "<<last.X()<<" "<<last.Y()<<" "<<last.Z();
        }
        res.push_back(list);
    }
    return res;
}

QVector<QPointF> Slice::discretizationOfEdge(const TopoDS_Edge &edge,float abscissa)const
{    
    QVector<QPointF> res;
    double dParamStart;
    double dParamEnd;
    Handle(Geom_Curve) aCurve = BRep_Tool::Curve(edge,dParamStart,dParamEnd);
    if (!aCurve.IsNull())
    {
        // Check the type of curve
        if (aCurve->IsKind(STANDARD_TYPE(Geom_Line)))
        {
            // This is the end of the cylinder
            gp_Pnt p1,p2;
            aCurve->D0(dParamStart,p1);
            aCurve->D0(dParamEnd,p2);
            QPointF v1 (p1.X(),p1.Y());
            QPointF v2 (p2.X(),p2.Y());
            //qDebug()<<v1<<" "<<v2;
            res.push_back(v1);
            res.push_back(v2);
        }
        else{
            BRepAdaptor_Curve curve_adaptator (edge);
            GCPnts_UniformAbscissa discretizer;
            discretizer.Initialize (curve_adaptator, abscissa);
            Q_ASSERT (discretizer.IsDone ());//computation_ok
            Q_ASSERT (discretizer.NbPoints () > 0);//positive_count_of_points

            for (int i = 1; i<=discretizer.NbPoints ();++i)
            {
                gp_Pnt p = curve_adaptator.Value (discretizer.Parameter (i));
                QPointF point (p.X (), p.Y ());
                res.push_back(point);
            }
        }
    }
    return res;
}

QString Slice::gp_PntToQString(gp_Pnt &p)
{
    return QString::number(p.X())+QString::number(p.Y())+QString::number(p.Z());
}

void Slice::updateProgressBar(int value)
{
    ui->progressBar->setValue(value);
}

bool Slice::exportSlice(const QString fileName, const QVector<Contour> &result)
{
    bool success;
    QFileInfo fileinfo(fileName);
    //文件后缀
    QString filesuffix = fileinfo.suffix();
    if(filesuffix == "slc")
    {
        success=ExportSLC(fileName,result);
    }
    else if(filesuffix == "cli"){
        success=ExportCLIBINARY(fileName,result);
    }
    Q_ASSERT(success);
    return success;
}

bool Slice::ExportCLIASCII(const QString fileName,const QVector<Contour> &result)
{
    if(result.empty())return false;
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"Open "<<fileName<<" failed.";
        return false;
    }
    QDateTime current_time = QDateTime::currentDateTime();
    QString timestr = current_time.toString( "yyMMdd"); //设置显示的格式
    QTextStream txtOutput(&f);
    txtOutput <<"$$HEADERSTART"<<endl;
    txtOutput <<"$$ASCII"<<endl;
    txtOutput <<"$$UNITS/1.000000"<<endl;
    txtOutput <<"$$VERSION/200 "<<endl;
    txtOutput <<"$$DATE/"+timestr<<endl;
    txtOutput <<"$$DIMENSION/"+QString::number(theXmin, 'f', 6) + ","
            + QString::number(theXmax, 'f', 6) + ","
            + QString::number(theYmin, 'f', 6) + ","
            + QString::number(theYmax, 'f', 6) + ","
            + QString::number(theZmin, 'f', 6) + ","
            + QString::number(theZmax, 'f', 6)<<endl;
    txtOutput <<"$$LAYERS/"+QString::number(result.size())<<endl;
    txtOutput <<"$$HEADEREND"<<endl;

    txtOutput <<"$$GEOMETRYSTART"<<endl;
    txtOutput <<"$$LAYER/"+QString::number(0.0)<<endl;
    for (uint i = 0; i < result.size(); i++) {
        Contour con = result[i];
        txtOutput <<"$$LAYER/"+QString::number(planeset[i].z())<<endl;
        uint numb_outerBoundaries=con.number_boundaries;
        for(uint j = 0; j < numb_outerBoundaries; j++) {
            uint numb_verts=uint(con.contour[j].getPolygon().size());
            if(con.contour[j].IsClockWise())
            {
                txtOutput <<"$$POLYLINE/0,"+QString::number(numb_verts)+",";
            }
            else{
                txtOutput <<"$$POLYLINE/1,"+QString::number(numb_verts)+",";
            }
            for(int k = 0; k < numb_verts; k++) {
                txtOutput<<QString::number(con.contour[j].getPolygon()[k].x())+",";
                txtOutput<<QString::number(con.contour[j].getPolygon()[k].y())+",";
            }
            txtOutput<<endl;
        }
    }
    txtOutput <<"$$GEOMETRYEND"<<endl;
    f.close();
    return true;
}

bool Slice::ExportCLIBINARY(const QString fileName, const QVector<Contour> &result)
{
    if(result.empty())return false;
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly))
    {
        qDebug()<<"Open "<<fileName<<" failed."<<endl;
        return false;
    }
    double translation_x = 100;
    double translation_y = 100;
    QDateTime current_time = QDateTime::currentDateTime();
    QString timestr = current_time.toString( "yyMMdd"); //设置显示的格式
    double unit = 0.005;//mm
    QDataStream out(&f);
    QString header =QString("$$HEADERSTART\n")
                    +"$$BINARY\n"
                    +"$$UNITS/"+QString::number(unit)+"\n"
                    +"$$VERSION/200\n"
                    +"$$LABEL/1,part1\n"
                    +"$$DATE/"+timestr+"\n"
                    +"$$DIMENSION/"+QString::number(theXmin, 'f', 6) + ","
            + QString::number(theXmax, 'f', 6) + ","
            + QString::number(theYmin, 'f', 6) + ","
            + QString::number(theYmax, 'f', 6) + ","
            + QString::number(theZmin, 'f', 6) + ","
            + QString::number(theZmax, 'f', 6)+"\n"
            +"$$LAYERS/"+QString::number(result.size())+"\n"
            +"$$HEADEREND";
    QByteArray  strBytes=header.toUtf8();//转换为字节数组
    f.write(strBytes,strBytes.length());  //写入文件
    strBytes.resize(2);
    uint16_t com;
    for (uint i = 0; i < result.size(); i++) {
        Contour con = result[i];
        com = 128;
        memcpy(strBytes.data(),&com,2);
        f.write(strBytes,2);
        com = con.getZ()/unit;
        memcpy(strBytes.data(),&com,2);
        f.write(strBytes,2);
        uint numb_outerBoundaries=con.number_boundaries;
        for(uint j = 0; j < numb_outerBoundaries; j++) {
            uint16_t numb_verts=uint16_t(con.contour[j].getPolygon().size());
            com = 129;
            memcpy(strBytes.data(),&com,2);
            f.write(strBytes,2);
            com = 1;
            memcpy(strBytes.data(),&com,2);
            f.write(strBytes,2);
            if(con.contour[j].IsClockWise())
            {
                com = 1;
                memcpy(strBytes.data(),&com,2);
                f.write(strBytes,2);
            }
            else{
                com = 1;
                memcpy(strBytes.data(),&com,2);
                f.write(strBytes,2);
            }
            memcpy(strBytes.data(),&numb_verts,2);
            f.write(strBytes,2);
            for(int k = 0; k < numb_verts; k++) {
                uint16_t x = (con.contour[j].getPolygon()[k].x()+translation_x)/unit;
                uint16_t y = (con.contour[j].getPolygon()[k].y()+translation_y)/unit;
                memcpy(strBytes.data(),&x,2);
                f.write(strBytes,2);
                memcpy(strBytes.data(),&y,2);
                f.write(strBytes,2);
                //cout<<k<<" "<<x<<" "<<QString::number(x, 16).toStdString()<<" "<<y<<" "<<QString::number(y, 16).toStdString()<<endl;
            }
        }
    }
    f.close();
    return true;
}

bool Slice::ExportSLC(const QString fileName,const QVector<Contour> &result)
{
    if(result.empty())return false;
    char buff[4];
    char charbuf[256];
    FILE *stream;
    QByteArray ba = fileName.toLocal8Bit();
    float value[2];
    if((stream=fopen(ba.data(),"wb"))==NULL)
    {
        std::cout<<"Cannot open output slc file."<<std::endl;
        return false;
    }

    QString slcHeader = "-SLCVER " + QString::number(2.0, 'f', 1)
            + " -UNIT " + "MM"
            + " -TYPE "  + "PART"
            + " -PACKAGE "  + "MATERIALISE C-TOOLS 2.xx"
            + " -EXTENTS " + QString::number(theXmin, 'f', 6) + ","
            + QString::number(theXmax, 'f', 6) + " "
            + QString::number(theYmin, 'f', 6) + ","
            + QString::number(theYmax, 'f', 6) + " "
            + QString::number(theZmin, 'f', 6) + ","
            + QString::number(theZmax, 'f', 6) + " ";

    fwrite(slcHeader.toLocal8Bit().data(), 1,slcHeader.toLocal8Bit().size(),stream);
    buff[0] = 0x0D;
    buff[1] = 0x0A;
    buff[2] = 0x1A;
    fwrite(buff,1,3,stream);
    memset(charbuf, 0x20, sizeof(charbuf));
    fwrite(charbuf, 1, sizeof(charbuf), stream);
    buff[0] =1;
    fwrite(buff, 1, 1, stream);
    float linewidecompen=0.025f;
    float reserved=0.025f;
    fwrite(&theZmin, sizeof(float), 1, stream);
    fwrite(&thickness, sizeof(float), 1, stream);
    fwrite(&linewidecompen, sizeof(float), 1, stream);
    fwrite(&reserved, sizeof(float), 1,  stream);
    for (uint i = 0; i < result.size(); i++) {
        Contour con = result[i];
        value[0] =con.getZ();
        fwrite(&value[0], sizeof(float), 1, stream);
        uint numb_outerBoundaries=con.number_boundaries;
        fwrite(&numb_outerBoundaries, sizeof(unsigned), 1, stream);
        for(uint j = 0; j < numb_outerBoundaries; j++) {
            uint numb_verts=uint(con.contour[j].getPolygon().size());
            uint numb_gaps=0;
            fwrite(&numb_verts, sizeof(unsigned), 1, stream);
            fwrite(&numb_gaps, sizeof(unsigned), 1, stream);
            for(int k = 0; k < numb_verts; k++) {
                value[0] = float(con.contour[j].getPolygon()[k].x());
                fwrite(&value[0], sizeof(float), 1, stream);
                value[0] = float(con.contour[j].getPolygon()[k].y());
                fwrite(&value[0], sizeof(float), 1,  stream);
            }
        }
    }

    float  maxZlays =result.back().getZ();
    fwrite(&maxZlays, sizeof(float), 1, stream);
    memset(buff, 0xff, 4);
    fwrite(buff,1,4,stream);
    fclose(stream);
    return true;
}

bool Slice::ChooseFilePath(QString &filename)
{
    filename = QFileDialog::getSaveFileName(this,tr("Save Slice"),"",tr("*.slc;; *.cli")); //选择路径
    if(filename.isEmpty())
    {
        return false;
    }
    return true;
}

LoopData::LoopData(QPolygonF polygon,bool isfill)
{
    _polygon = polygon;
    _isfill = isfill;
}

bool LoopData::IsClockWise()
{
    int d = 0;
    int N = _polygon.size();
    for (int i = 0; i < N - 1; i++)
        d += -0.5 * (_polygon[i + 1].y() + _polygon[i].y()) * (_polygon[i + 1].x() - _polygon[i].x());
    return (d<=0);
}

double LoopData::getArea()
{
    return ComputePolygonArea();
}

QPolygonF LoopData::getPolygon()
{
    return _polygon;
}
bool LoopData::IsSelfIntersection()
{
    return CheckSelfIntersection();
}

double LoopData::ComputePolygonArea()
{
    int point_num = _polygon.size()-1;
    if(point_num < 3)return 0.0;
    double s = _polygon[0].y() * (_polygon[point_num-1].x() - _polygon[1].x());
    for(int i = 1; i < point_num; ++i)
        s += _polygon[i].y() * (_polygon[i-1].x() - _polygon[(i+1)%point_num].x());
    return abs(s/2.0);
}

bool LoopData::CheckSelfIntersection()
{
    std::vector<QLineF> segments;
    if(!_polygon.isClosed()) return true;
    int N = _polygon.size();
    segments.resize(N-1);
    for (int i = 0; i < N - 1; i++)
    {
        segments[i] = QLineF(_polygon[i],_polygon[i+1]);
    }
    QPointF point;
    for (uint i=0;i<segments.size();++i) {
        for (uint j=i+1;j<segments.size();++j) {
            QLineF::IntersectType type = segments[i].intersect(segments[j],&point);
            if(type == QLineF::BoundedIntersection && point != segments[i].p1() && point != segments[i].p2())
                return true;
        }
    }
    return false;
}

bool LoopData::smooth()
{
    qreal maxlength = 0.0;
    int N =_polygon.isClosed() ? _polygon.size(): _polygon.size()-1;
    for (int i=0;i<N;i++) {
        int pre = i==0 ? N-1 : i-1;
        int next = i+1;
        QLineF prel(_polygon[pre],_polygon[i]);
        QLineF nextl(_polygon[i],_polygon[next]);
        QLineF oppol(_polygon[next],_polygon[pre]);

    }
    return true;
}

bool Contour::IsIsSelfIntersection()
{
    for (LoopData loop : contour) {
        if(loop.IsSelfIntersection())
            return true;
    }
    return false;
}
