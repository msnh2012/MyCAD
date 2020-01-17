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
#include <TopExp_Explorer.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include "View.h"
#include <QElapsedTimer>
Slice::Slice(QWidget *parent):
    QDialog(parent),ui(new Ui::Slice)
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
    ui->isParallel->setVisible(false);
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
        if(ui->isParallel->isChecked())
        {
            #pragma omp parallel for num_threads(8)
            for (int l=0;l<numberlayer;l++) {
                gp_Pln plane(gp_Pnt(planeset[l].x(),planeset[l].y(),planeset[l].z()),normal);
                BRepAlgoAPI_Section S(shape,plane,Standard_False);
                S.Approximation(Standard_True);
                S.Build();
                S.SetRunParallel(true);
                TopTools_ListOfShape edges = S.SectionEdges();
                QVector<QList<TopoDS_Edge>> layer=splitEdges(edges);
                slicedata.append(SliceData(S.Shape(),layer,plane,thickness));
                int fraction = interval*(i-1+float(l)/numberlayer);
                //emit sglStep(fraction);
            }
        }
        else{
            for (int l=0;l<numberlayer;l++) {
                gp_Pln plane(gp_Pnt(planeset[l].x(),planeset[l].y(),planeset[l].z()),normal);
                BRepAlgoAPI_Section S(shape,plane,Standard_False);
                S.Approximation(Standard_True);
                S.Build();
                S.SetRunParallel(true);
                TopTools_ListOfShape edges = S.SectionEdges();
                QVector<QList<TopoDS_Edge>> layer=splitEdges(edges);
                slicedata.append(SliceData(S.Shape(),layer,plane,thickness));
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
    double  	theXmin;
    double  	theYmin;
    double  	theZmin;
    double  	theXmax;
    double  	theYmax;
    double  	theZmax;
    double min,max;
    box.Get(theXmin,theYmin,theZmin,theXmax,theYmax,theZmax);
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
    myContext->Display( new AIS_Shape( slicedata.at(value).edges ), false );
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
    int j = ui->comboBox->currentIndex();
    QVector<SliceData> slicedata =doc->myComponents[j]->getSlice();
    for ( int i = 0; i < 1; i++ )
    {
        gp_Pnt first,last;
        QVector<QList<TopoDS_Edge>> layer = slicedata[i].layer;
        for (int i=0;i<layer.size();i++) {
            QList<TopoDS_Edge> con = layer[i];
            for (TopoDS_Edge edge : con) {
                getEndpointOfEdge(edge,first,last);
                qDebug()<<first.X()<<" "<<first.Y()<<" "<<first.Z()<<" "<<last.X()<<" "<<last.Y()<<" "<<last.Z();
            }
        }
    }

}

QVector<QList<TopoDS_Edge>> Slice::splitEdges(TopTools_ListOfShape &edges)
{
    QVector<QList<TopoDS_Edge>> res;
    QMultiMap<QString,TopoDS_Edge> hashmap;
    QList<TopoDS_Edge> con;
    for (TopoDS_Shape edge :edges) {
        TopoDS_Edge e = TopoDS::Edge(edge);
        if(e.Closed())
        {
            con.clear();
            con.append(e);
            res.append(con);
        }
        else
        {
            gp_Pnt first,last;
            getEndpointOfEdge(e,first,last);
            //qDebug()<<first.X()<<" "<<first.Y()<<" "<<first.Z()<<" "<<last.X()<<" "<<last.Y()<<" "<<last.Z();
            hashmap.insert(gp_PntToQString(first),e);
            hashmap.insert(gp_PntToQString(last),e);
        }
    }

    gp_Pnt first,last;
    while(!hashmap.empty())
    {
        con.clear();
        TopoDS_Edge curE =hashmap.first();
        con.append(curE);
        getEndpointOfEdge(curE,first,last);
        QString curP = gp_PntToQString(first);
        hashmap.remove(curP,curE);
        while(1){
            auto find_index = hashmap.find(curP);
            if(find_index!=hashmap.end()) {
                curE = find_index.value();
                hashmap.remove(curP,curE);
                getEndpointOfEdge(curE,first,last);
                if(curP == gp_PntToQString(first))
                {
                    curP =  gp_PntToQString(last);

                }
                else{
                    curP =  gp_PntToQString(first);
                }
                hashmap.remove(curP,curE);
                con.append(curE);
                if(con.front() == curE)
                {
                    res.push_back(con);
                    break;
                }
            }
            else{
                res.push_back(con);
                break;
            }

        }
//        for (TopoDS_Edge edge : con) {
//            getEndpointOfEdge(edge,first,last);
//            qDebug()<<first.X()<<" "<<first.Y()<<" "<<first.Z()<<" "<<last.X()<<" "<<last.Y()<<" "<<last.Z();
//        }
    }
    return res;
}

void Slice::getEndpointOfEdge(TopoDS_Edge &edge, gp_Pnt & first, gp_Pnt &last)
{
    Standard_Real First,Last;
    TopLoc_Location L;
    Handle( Geom_Curve) curve = BRep_Tool::Curve(edge,First,Last);
    curve->D0(First, first);
    curve->D0(Last, last);
}

QString Slice::gp_PntToQString(gp_Pnt &p)
{
    return QString::number(p.X())+QString::number(p.Y())+QString::number(p.Z());
}

void Slice::updateProgressBar(int value)
{
    ui->progressBar->setValue(value);
}
