
#include "GaudiKernel/DeclareFactoryEntries.h"

#include "../MyAlg.h"

DECLARE_ALGORITHM_FACTORY( MyAlg )

DECLARE_FACTORY_ENTRIES( MyPackage ) 
{
  DECLARE_ALGORITHM( MyAlg );
}
