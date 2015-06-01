#include "hslider.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QLinearGradient>

#include <QtCore>

#include "Service/services.h"


// --------------------------------------------- consts ---------------------------------------------

const int maxH=360;
const int maxSV=255;
const double maxHF=1.0;
const double maxSVF=1.0;

const double ratio=1.0/maxH;

const int border=2;
const int borderRadius=10;

const int sliderX=border;
const int sliderY=border;
const int barWidth=40;

const int pointerWidth=10;
const int pointerDy1=1.5;
const int pointerDy2=5;

const QColor borderColor(80,80,80,200);

const QColor pointerColor("#333");

// ---

const int pointerR=5;
const int pointerBorder=2;

const QColor pointerLineColor(80,80,80,200);
const QColor pointerBorderColor("#ddd");
const QColor pointerFillColor("#333");


// ----------------------------------------------------------------------------------------------------

HSlider::HSlider(QWidget *parent) :
  QWidget(parent)
{
  hSliderDrawn=false;
  middlePresed=false;
  ctrlHeld=false;
  widthChanged=false;
  
  pointerX=0;
  pointerY=0;

  sliderVal=0;
  
  h=0;
  s=maxSVF;
  v=maxSVF;
}

void HSlider::paintEvent(QPaintEvent *e)
{
  QColor color;
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  
  calcVars();

  if(!hSliderDrawn || widthChanged){
    hSliderPixmap=QPixmap(sliderW, sliderH);
    QPainter tempP( &hSliderPixmap );

    QPointF p1( sliderX, sliderH/2 );
    QPointF p2( sliderX+sliderW, sliderH/2 );
    QLinearGradient grad(p1, p2);

    for(double hs=0; hs<1.0; hs+=ratio){
      color.setHsvF(hs, 1, 1);
      grad.setColorAt(hs, color);
    }

    tempP.setPen(Qt::NoPen);
    tempP.setBrush( QBrush(grad) );
    tempP.drawRect(0, 0, sliderW, sliderH);

    hSliderDrawn=true;
    widthChanged=false;
  }
  p.drawPixmap(sliderX, sliderY, hSliderPixmap);

  drawPointer(p);
  drawBorder(p);
}

void HSlider::calcVars(){
  sliderX=border;
  sliderY=border;
  sliderW=width()-2*border;
  sliderH=height()-2*border;

  if(sliderW!=prevSliderW){
    widthChanged=true;
//    int x=normalizePointerX(h);
//    sliderVal=x;
  }
  prevSliderW=sliderW;

  maxRange = sliderW - 1;
  minPointerX = sliderX;
  maxPointerX = minPointerX + maxRange;
}

void HSlider::mousePressEvent(QMouseEvent *e)
{
  if( e->button() == Qt::MiddleButton ){
    middlePresed=true;
  }
  else{
    middlePresed=false;
    movePointer( e->x() );
    updateColor();
  }
}

void HSlider::mouseMoveEvent(QMouseEvent *e)
{
  if(middlePresed){
    e->ignore();
  }
  else{
    movePointer( e->x() );
    updateColor();
  }
}

void HSlider::mouseReleaseEvent(QMouseEvent *e)
{
}

void HSlider::wheelEvent(QWheelEvent *e)
{
  QPoint p=e->angleDelta();
  
  int val=1;
  if(ctrlHeld) val=10;
  
  if(p.y()>0) incPointer(val);
  if(p.y()<0) decPointer(val);

  updateColor();
  e->accept();
}


// ---------------------------------------------- service ----------------------------------------------

void HSlider::drawPointer(QPainter& p){
  correctPointer();

  pointerY = height()/2;

  QPen pen(pointerBorderColor, pointerBorder);
  p.setPen(pen);
  p.setBrush(pointerFillColor);
  p.drawEllipse( QPoint(pointerX, pointerY), pointerR, pointerR );

  pen.setColor(pointerLineColor);
  pen.setCapStyle(Qt::FlatCap);
  p.setPen(pen);

  p.drawLine( QPoint(pointerX, sliderY), QPoint(pointerX, pointerY-pointerR-pointerBorder/2) );
  p.drawLine( QPoint(pointerX, sliderY+sliderH), QPoint(pointerX, pointerY+pointerR+pointerBorder/2) );
}

int HSlider::normalizePointerX(double val){
  double d=val*maxRange;
  return qRound(d);

//  for(int x=0; x<=maxRange; ++x){
//    if( normalizeH(x) == h ){
//      return x;
//    }
//  }
//  return 0;
}

double HSlider::normalizeH(int val){
  double h = (double)val/maxRange;
  return h;
}

void HSlider::correctPointer(){
  pointerX = sliderVal + minPointerX;
  pointerX = qMax( minPointerX, pointerX );
  pointerX = qMin( maxPointerX, pointerX );
}

void HSlider::updateColor(){
  QColor color;

  sliderVal = pointerX - minPointerX;
  sliderVal = qMax(0, sliderVal);
  sliderVal = qMin(maxRange, sliderVal);

  h = normalizeH( sliderVal );
  h = qMax(0.0, h);
  h = qMin(maxHF, h);
  
  color.setHsvF(h, s, v);
  emit hueChanged(color);
}

void HSlider::log(QString format, QVariant var){
//  qDebug() << QString(format).arg(var);
}

void HSlider::incPointer(int val){
  movePointer(pointerX+val);
}

void HSlider::decPointer(int val){
  movePointer(pointerX-val);
}

void HSlider::movePointer(int x){
  emit pointerMoved(x);
  pointerX=x;
  update();
}

void HSlider::drawBorder(QPainter& p){
  QRectF rectangle( sliderX, sliderY, sliderW, sliderH );                               // set inner rect coordinates (the border will be outer)
  Services::drawRoundRect( p, rectangle, border, borderRadius, borderColor );
}

void HSlider::hideCursor(QMouseEvent *e){
  int x=e->x();
  int y=e->y();

  if( ( x<minPointerX || x>maxPointerX ) || ( y<sliderY || y>sliderY+sliderH ) ) {
    restoreCursor();
    return;
  }
  this->setCursor( QCursor(Qt::BlankCursor) );
}

void HSlider::restoreCursor(){
  this->setCursor( QCursor(Qt::ArrowCursor) );
}


// ---------------------------------------------- set/get ----------------------------------------------

void HSlider::setH(int h, double hf)
{
  double d=(double) h/maxH;
  sliderVal=qCeil(hf*maxRange);
  
  this->h=hf;
  QColor color;
  color.setHsvF(hf, s, v);

//  emit hueChanged(color);
  update();
}

// ---------------------------------------------- slots ----------------------------------------------

void HSlider::ctrlPressed(){
  ctrlHeld=true;
}

void HSlider::ctrlReleased(){
  ctrlHeld=false;
}
