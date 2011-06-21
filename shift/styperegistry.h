#ifndef STYPEREGISTRY_H
#define STYPEREGISTRY_H

#include "sglobal.h"
#include "XHash"
#include "spropertyinformation.h"

class SHIFT_EXPORT STypeRegistry
  {
public:
  static void initiate();

  static const XHash <SPropertyType, const SPropertyInformation *> &types();
  static void addType(const SPropertyInformation *);

  static const SPropertyInformation *findType(const QString &);
  static const SPropertyInformation *findType(xuint32);

private:
  STypeRegistry();
  X_DISABLE_COPY(STypeRegistry)
  };

#endif // STYPEREGISTRY_H
