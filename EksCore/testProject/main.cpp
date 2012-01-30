#include "QDebug"
#include "Eigen/Core"
#include "Eigen/Geometry"
#include "XArrayMath"


int main()
  {
  float arrData[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  XMathsOperation arr;
  arr.load(XMathsOperation::Float, (xuint8*)arrData, 1, sizeof(arrData)/sizeof(arrData[0]), 1);

  float arrData2[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
  XMathsOperation arr2;
  arr.load(XMathsOperation::Float, (xuint8*)arrData2, 1, sizeof(arrData2)/sizeof(arrData2[0]), 1);

  XMathsOperation add;
  add.add(arr, arr2);

  XMathsOperation convolve;
  float convData[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  XMathsOperation conv;
  conv.load(XMathsOperation::Float, (xuint8*)convData, 1, sizeof(convData)/sizeof(convData[0]), 1);
  convolve.convolve(add, conv);

  XMathsOperation shuffle;
  xuint32 mask;
  mask |= 1;
  mask |= 0 << 8;
  mask |= 2 << 16;
  mask |= 3 << 24;
  shuffle.shuffle(convolve, mask);

  XMathsOperation mul;
  mul.multiply(shuffle, arr);

  XMathsResult res(mul);
  float* data = (float*)res.data();
  xsize length = res.dataWidth();

  for(xsize i=0; i < length; ++i)
    {
    qDebug() << data[i];
    }
  }

/*
int main( )
  {
  Eigen::VectorXf a(3);
  Eigen::VectorXf b(a);
  Eigen::VectorXf c(b);

  a = Eigen::Vector4f(1, 2, 3, 4);
  b = Eigen::Vector4f(3, 2, 1, 0);

  c = a + b;

  Eigen::Matrix<float,Eigen::Dynamic,Eigen::Dynamic> x(1,3);
  Eigen::Matrix<float,Eigen::Dynamic,Eigen::Dynamic> y(3,1);

  x = y;

  x.resize(4,4);
  x.resize(2,3);

  y = x;

  y = a;

  Eigen::Vector3f d = a.block<3, 1>(0, 0);

  c = a;

  Eigen::Vector3f e = a.block<3, 1>(0, 0);

  Eigen::ArrayXf ddd = c;

  ddd = a;

  Eigen::Matrix3f f;

  f.row(0) = e;

  Eigen::Affine3f m;


  m.matrix().row(0).head<3>() = e;

  return EXIT_SUCCESS;
  }
*/