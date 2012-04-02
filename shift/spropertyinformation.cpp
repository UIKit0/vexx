#include "spropertyinformation.h"
#include "spropertycontainer.h"
#include "sdatabase.h"
#include "styperegistry.h"

SPropertyInstanceInformation::SPropertyInstanceInformation()
  {
  _holdingTypeInformation = 0;
  _compute = 0;
  _location = 0;
  _computeLockedToMainThread = false;
  _queueCompute = defaultQueue;
  _affects = 0;
  _mode = Default;
  _extra = false;
  _dynamic = false;
  _defaultInput = 0;
  }

SPropertyInstanceInformation *SPropertyInstanceInformation::allocate(xsize size)
  {
  xAssert(STypeRegistry::allocator());
  void *ptr = STypeRegistry::allocator()->alloc(size);

  xAssert(ptr);
  return (SPropertyInstanceInformation*)ptr;
  }

void SPropertyInstanceInformation::destroy(SPropertyInstanceInformation *d)
  {
  xAssert(STypeRegistry::allocator());
  STypeRegistry::allocator()->free(d);
  }

SPropertyInformation *SPropertyInformation::allocate()
  {
  xAssert(STypeRegistry::allocator());
  void *ptr = STypeRegistry::allocator()->alloc(sizeof(SPropertyInformation));

  xAssert(ptr);
  return new(ptr) SPropertyInformation;
  }

void SPropertyInformation::destroy(SPropertyInformation *d)
  {
  foreach(SInterfaceBaseFactory *f, d->_interfaceFactories)
    {
    xAssert(f->_referenceCount > 0);
    --f->_referenceCount;
    if(f->_referenceCount == 0 && f->_deleteOnNoReferences)
      {
      delete f;
      }
    }

  for(xsize i=0; i<d->childCount(); ++i)
    {
    SPropertyInstanceInformation *inst = d->child(i);
    delete [] inst->affects();
    inst->~SPropertyInstanceInformation();
    SPropertyInstanceInformation::destroy(inst);
    }

  xAssert(STypeRegistry::allocator());
  STypeRegistry::allocator()->free(d);
  }

void SPropertyInformation::initiate(SPropertyInformation *info, const SPropertyInformation *from)
  {
  // update template constructor too
  info->setCreateProperty(from->createProperty());
  info->setSave(from->save());
  info->setLoad(from->load());
  info->setShouldSave(from->shouldSave());
  info->setShouldSaveValue(from->shouldSaveValue());
  info->setAssign(from->assign());
  info->setPostCreate(from->postCreate());
  info->setPostChildSet(from->postChildSet());
  info->setVersion(from->version());
  info->setSize(from->size());
  info->setInstanceInformationSize(from->instanceInformationSize());

  info->_createInstanceInformation = from->_createInstanceInformation;
  info->_derive = from->_derive;
  info->_instances = 0;
  info->_extendedParent = 0;

  info->_typeName = from->typeName();
  }

SPropertyInformation *SPropertyInformation::derive(const SPropertyInformation *from)
  {
  xAssert(from->_derive);
  SPropertyInformation *copy = SPropertyInformation::allocate();

  SPropertyInformation::initiate(copy, from);

  SPropertyInformationCreateData data;
  data.registerAttributes = true;
  data.registerInterfaces = false;

  from->_derive(copy, data);
  copy->setParentTypeInformation(from);

  xAssert(copy);
  return copy;
  }

SPropertyInstanceInformation *SPropertyInformation::add(const SPropertyInformation *newChildType, const QString &name)
  {
  xsize backwardsOffset = 0;
  SPropertyInformation *allocatable = findAllocatableBase(backwardsOffset);
  xAssert(allocatable);

  xsize finalLocation = X_ROUND_TO_ALIGNMENT(xsize, allocatable->size());
  xsize finalSize = finalLocation + newChildType->size();

  allocatable->setSize(finalSize);

  xAssert(finalLocation > backwardsOffset);
  xsize location = finalLocation - backwardsOffset;

  SPropertyInstanceInformation *def = add(newChildType, location, name, true);

  const SProperty *prop = def->locateProperty((const SPropertyContainer*)0);
  xAssert((backwardsOffset + (xsize)prop) == finalLocation);

  return def;
  }

SPropertyInstanceInformation *SPropertyInformation::add(const SPropertyInformation *newChildType, xsize location, const QString &name, bool extra)
  {
  xAssert(newChildType);
  SPropertyInstanceInformation *def = newChildType->_createInstanceInformation(name, _children.size(), location);

  def->setHoldingTypeInformation(this);
  def->setExtra(extra);

  _children << def;

  return def;
  }

SPropertyInformation *SPropertyInformation::extendContainedProperty(SPropertyInstanceInformation *inst)
  {
  const SPropertyInformation *oldInst = inst->childInformation();
  SPropertyInformation *info = SPropertyInformation::derive(oldInst);

  info->setExtendedParent(inst);
  inst->setChildInformation(info);

  return info;
  }


SPropertyInstanceInformation *SPropertyInformation::child(SProperty SPropertyContainer::* ptr)
  {
  SPropertyContainer *u = 0;
  SProperty *offset = &(u->*ptr);
  xptrdiff location = reinterpret_cast<xsize>(offset);
  xAssert(location > 0);
  return child(location);
  }

const SPropertyInstanceInformation *SPropertyInformation::child(SProperty SPropertyContainer::* ptr) const
  {
  SPropertyContainer *u = 0;
  SProperty *offset = &(u->*ptr);
  xptrdiff location = reinterpret_cast<xsize>(offset);
  xAssert(location > 0);
  return child(location);
  }

SPropertyInstanceInformation *SPropertyInformation::child(xsize location)
  {
  foreach(SPropertyInstanceInformation *i, _children)
    {
    if(i->location() == location)
      {
      return i;
      }
    }
  return 0;
  }

const SPropertyInstanceInformation *SPropertyInformation::child(xsize location) const
  {
  foreach(const SPropertyInstanceInformation *i, _children)
    {
    if(i->location() == location)
      {
      return i;
      }
    }
  return 0;
  }

const SPropertyInstanceInformation *SPropertyInformation::childFromIndex(xsize index) const
  {
  xAssert(index < childCount());
  return _children[index];
  }

SPropertyInstanceInformation *SPropertyInformation::childFromIndex(xsize index)
  {
  xAssert(index < childCount());
  return _children[index];
  }

const SPropertyInstanceInformation *SPropertyInformation::childFromName(const QString &in) const
  {
  foreach(SPropertyInstanceInformation *i, _children)
    {
    if(i->name() == in)
      {
      return i;
      }
    }
  return 0;
  }

SPropertyInstanceInformation *SPropertyInformation::childFromName(const QString &in)
  {
  foreach(SPropertyInstanceInformation *i, _children)
    {
    if(i->name() == in)
      {
      return i;
      }
    }
  return 0;
  }

const SPropertyInformation *SPropertyInformation::findAllocatableBase(xsize &offset) const
  {
  offset = 0;

  const SPropertyInstanceInformation *allocateOnInfo = extendedParent();
  if(!allocateOnInfo)
    {
    return this;
    }

  const SPropertyInformation *allocateOn = this;
  while(allocateOnInfo)
    {
    offset += allocateOnInfo->location();

    const SPropertyInformation *holdingInfo = allocateOnInfo->holdingTypeInformation();
    if(!holdingInfo)
      {
      return allocateOn;
      }

    allocateOn = holdingInfo;
    allocateOnInfo = allocateOn->extendedParent();
    }

  return 0;
  }

SPropertyInformation *SPropertyInformation::findAllocatableBase(xsize &offset)
  {
  offset = 0;

  const SPropertyInstanceInformation *allocateOnInfo = extendedParent();
  if(!allocateOnInfo)
    {
    return this;
    }

  SPropertyInformation *allocateOn = this;
  while(allocateOnInfo)
    {
    offset += allocateOnInfo->location();

    SPropertyInformation *holdingInfo = allocateOnInfo->holdingTypeInformation();
    if(!holdingInfo)
      {
      return allocateOn;
      }

    allocateOn = holdingInfo;
    allocateOnInfo = allocateOn->extendedParent();
    }

  return allocateOn;
  }

SPropertyInformation::DataKey g_maxKey = 0;
SPropertyInformation::DataKey SPropertyInformation::newDataKey()
  {
  return g_maxKey++;
  }

void SPropertyInformation::setData(DataKey k, const QVariant &v)
  {
  xAssert(k < g_maxKey);
  _data[k].setValue(v);
  }

SPropertyInstanceInformation::DataKey g_maxChildKey = 0;
SPropertyInstanceInformation::DataKey SPropertyInstanceInformation::newDataKey()
  {
  return g_maxChildKey++;
  }

QString g_modeStrings[] = {
  /* Internal      */ "internal",
  /* InputOutput   */ "inputoutput",
  /* InternalInput */ "internalinput",
  /* Input         */ "input",
  /* Output        */ "output",
  /* Computed      */ "computed",
  /* InternalComputed */ "internalcomputed",
  /* UserSettable  */ "usersettable"
};

const QString &SPropertyInstanceInformation::modeString() const
  {
  xAssert(_mode < NumberOfModes);
  return g_modeStrings[_mode];
  }

void SPropertyInstanceInformation::setMode(Mode m)
  {
  if(_compute)
    {
    if(m == Internal)
      {
      _mode = InternalComputed;
      }
    else
      {
      xAssertFail();
      }
    }
  else
    {
    if(!_compute)
      {
      _mode = m;
      }
    }
  }

void SPropertyInstanceInformation::setModeString(const QString &s)
  {
  for(xsize i = 0; i < NumberOfModes; ++i)
    {
    if(g_modeStrings[i] == s)
      {
      _mode = (Mode)i;
      return;
      }
    }

  _mode = Default;
  }

bool SPropertyInstanceInformation::isDefaultMode() const
  {
  return _mode == Default;
  }

void SPropertyInstanceInformation::setCompute(ComputeFunction fn)
  {
  _compute = fn;
  if(_compute)
    {
    _mode = Computed;
    }
  }

void SPropertyInstanceInformation::setAffects(const SPropertyInstanceInformation *info)
  {
  xAssert(!_affects);
  xAssert(info);

  _affects = new xsize[2];
  _affects[0] = info->location();
  _affects[1] = 0;
  }

void SPropertyInstanceInformation::setAffects(const SPropertyInstanceInformation **info, xsize size)
  {
  xAssert(!_affects);
  xAssert(info);

  _affects = new xsize[size+1];

  for(xsize i = 0; i < size; ++i)
    {
    _affects[i] = info[i]->location();
    }

  _affects[size] = 0;
  }

void SPropertyInstanceInformation::setAffects(xsize *affects)
  {
  xAssert(!_affects);
  _affects = affects;
  }

void SPropertyInstanceInformation::setDefaultInput(const SPropertyInstanceInformation *info)
  {
  // find the offset to the holding type information
  xsize targetOffset = 0;
  const SPropertyInformation *targetBase = info->holdingTypeInformation()->findAllocatableBase(targetOffset);
  // add the instance location
  targetOffset += info->location();

  // find the offset to the holding type information
  xsize sourceOffset = 0;
  const SPropertyInformation *sourceBase = holdingTypeInformation()->findAllocatableBase(sourceOffset);
  // add the instance location
  sourceOffset += location();

  // cannot add a default input between to separate allocatable types.
  xAssert(targetBase == sourceBase);

  _defaultInput = (xptrdiff)targetOffset - (xptrdiff)sourceOffset;

  xAssert(sourceOffset < sourceBase->size());
  xAssert(targetOffset < sourceBase->size());
  xAssert((sourceOffset + _defaultInput) < sourceBase->size());
  xAssert((targetOffset - _defaultInput) < sourceBase->size());
  xAssert(_defaultInput < (xptrdiff)sourceBase->size());
  }

void SPropertyInstanceInformation::initiateProperty(SProperty *propertyToInitiate) const
  {
  if(defaultInput())
    {
    xuint8 *data = (xuint8*)propertyToInitiate;

    const xuint8 *inputPropertyData = data + defaultInput();

    const SProperty *inputProperty = (SProperty*)inputPropertyData;

    inputProperty->connect(propertyToInitiate);
    }
  }

void SPropertyInstanceInformation::initiate(const SPropertyInformation *info,
                 const QString &name,
                 xsize index,
                 xsize location)
  {
  _childInformation = info;
  _name = name;
  _location = location;
  _index = index;
  }

void SPropertyInstanceInformation::setData(DataKey k, const QVariant &v)
  {
  xAssert(k < g_maxChildKey);
  _data[k].setValue(v);
  }

SProperty *SPropertyInstanceInformation::locateProperty(SPropertyContainer *parent) const
  {
  xuint8* parentOffset = reinterpret_cast<xuint8*>(parent);
  xuint8* childOffset = parentOffset + location();
  SProperty *child = reinterpret_cast<SProperty*>(childOffset);
  return child;
  }

const SProperty *SPropertyInstanceInformation::locateProperty(const SPropertyContainer *parent) const
  {
  const xuint8* parentOffset = reinterpret_cast<const xuint8*>(parent);
  const xuint8* childOffset = parentOffset + location();
  const SProperty *child = reinterpret_cast<const SProperty*>(childOffset);
  return child;
  }

const SPropertyInstanceInformation *SPropertyInstanceInformation::resolvePath(const QString &path) const
  {
  SProfileFunction

  const SPropertyInstanceInformation *cur = this;
  const SPropertyInformation *curInfo = cur->childInformation();

  QString name;
  bool escape = false;
  for(xsize i = 0, s = path.size(); i < s; ++i)
    {
    QChar c = path[i];

    if(c == QChar('\\'))
      {
      escape = true;
      }
    else
      {
      if(!escape && c != SDatabase::pathSeparator())
        {
        name.append(c);
        }

      if(!escape && (c == SDatabase::pathSeparator() || i == (s-1)))
        {
        if(name == "..")
          {
          xAssert(cur);
          if(!cur)
            {
            return 0;
            }

          curInfo = cur->holdingTypeInformation();
          if(!curInfo)
            {
            return 0;
            }

          cur = curInfo->extendedParent();
          }
        else
          {
          xAssert(curInfo);
          cur = curInfo->childFromName(name);
          if(!cur)
            {
            return 0;
            }

          curInfo = cur->childInformation();
          }

        if(!cur && !curInfo)
          {
          return 0;
          }
        xAssert(curInfo);

        name.clear();
        }
      escape = false;
      }
    }
  return cur;
  }

void SPropertyInstanceInformation::defaultQueue(const SPropertyInstanceInformation *info, const SPropertyContainer *cont, SProperty **jobs, xsize &numJobs)
  {
  SProfileFunction
  for(SProperty *prop=cont->firstChild(); prop; prop=prop->nextSibling())
    {
    const SPropertyInstanceInformation *siblingInfo = prop->instanceInformation();
    if(siblingInfo->affects() && !siblingInfo->computeLockedToMainThread())
      {
      const SPropertyInformation *contInfo = cont->typeInformation();
      xsize i=0;
      while(siblingInfo->affects()[i])
        {
        const SPropertyInstanceInformation *affectsInst = contInfo->child(siblingInfo->affects()[i]);
        const SProperty *thisProp = affectsInst->locateProperty(cont);
        const SPropertyInstanceInformation *thisInfo = thisProp->instanceInformation();
        if(thisInfo == info)
          {
          jobs[numJobs++] = prop;
          }
        ++i;
        }
      }
    }
  }

bool SPropertyInformation::inheritsFromType(const SPropertyInformation *match) const
  {
  const SPropertyInformation *type = this;
  while(type)
    {
    if(type == match)
      {
      return true;
      }
    type = type->parentTypeInformation();
    }
  return false;
  }

void SPropertyInformation::reference() const
  {
  ++((SPropertyInformation*)this)->_instances;
  }

void SPropertyInformation::dereference() const
  {
  --((SPropertyInformation*)this)->_instances;
  }
