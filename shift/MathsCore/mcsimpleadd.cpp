#include "mcsimpleadd.h"

void computeAddOutput(const SPropertyInstanceInformation *, MCSimpleAdd *add)
  {
  FloatProperty::ComputeLock l(&add->output);

  *l.data() = add->inputA() + add->inputB();
  }

S_IMPLEMENT_PROPERTY(MCSimpleAdd)

void MCSimpleAdd::createTypeInformation(SPropertyInformation *info, const SPropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    FloatProperty::InstanceInformation *outInst = info->child(&MCSimpleAdd::output);
    outInst->setCompute(computeAddOutput);
    }
  }

MCSimpleAdd::MCSimpleAdd()
  {
  }