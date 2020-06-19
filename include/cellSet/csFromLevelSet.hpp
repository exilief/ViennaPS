#ifndef CS_FROM_LEVEL_SET_HPP
#define CS_FROM_LEVEL_SET_HPP

// #include <set>
// #include <unordered_set>

#include <hrleFillDomainFromPointList.hpp>
#include <hrleFillDomainWithSignedDistance.hpp>
#include <hrleVectorType.hpp>

#include <lsCalculateNormalVectors.hpp>
// #include <lsConvexHull.hpp>

#include <csDomain.hpp>

/// Enumeration for the different types of conversion
enum struct csFromLevelsetEnum : unsigned {
  ANALYTICAL = 0,
  LOOKUP = 1,
  SIMPLE = 2,
};

/// This class constructs a csDomain from an lsDomain.
/// Filling fraction can be calculated by cutting each voxel
/// by the plane of an LS disk (normal and ls value) and computing the volume.
template <class LSType, class CSType> class csFromLevelSet {
  LSType *levelSet = nullptr;
  CSType *cellSet = nullptr;
  csFromLevelsetEnum conversionType = csFromLevelsetEnum::SIMPLE; // TODO change to lookup
  bool calculateFillingFraction = true;
  static constexpr int D = LSType::dimensions;

  void convertSimple() {
    // typedef typename CSType::ValueType CellType;
    auto &grid = levelSet->getGrid();
    CSType newCSDomain(grid, cellSet->getBackGroundValue(), cellSet->getEmptyValue());
    typename CSType::DomainType &newDomain = newCSDomain.getDomain();
    typename LSType::DomainType &domain = levelSet->getDomain();

    newDomain.initialize(domain.getNewSegmentation(), domain.getAllocation());

// go over each point and calculate the filling fraction
#pragma omp parallel num_threads(domain.getNumberOfSegments())
    {
      int p = 0;
#ifdef _OPENMP
      p = omp_get_thread_num();
#endif

      hrleVectorType<hrleIndexType, D> startVector =
          (p == 0) ? grid.getMinGridPoint() : domain.getSegmentation()[p - 1];

      hrleVectorType<hrleIndexType, D> endVector =
          (p != static_cast<int>(domain.getNumberOfSegments() - 1))
              ? domain.getSegmentation()[p]
              : grid.incrementIndices(grid.getMaxGridPoint());

      for (hrleConstSparseIterator<typename LSType::DomainType> it(domain,
                                                                   startVector);
           it.getStartIndices() < endVector; it.next()) {

        // skip this voxel if there is no plane inside
        if (!it.isDefined() || std::abs(it.getValue()) > 0.5) {
          auto undefinedValue = (it.getValue() > 0)
                                    ? cellSet->getEmptyValue()
                                    : cellSet->getBackGroundValue();
          // insert an undefined point to create correct hrle structure
          newDomain.insertNextUndefinedPoint(p, it.getStartIndices(),
                                             undefinedValue);
          continue;
        }

        typename CSType::ValueType cell;

        // this is the function to convert an LS value to a CS value
        cell.setFillingFraction(0.5 - it.getValue());

        newDomain.insertNextDefinedPoint(p, it.getStartIndices(), cell);
      } // end of ls loop
    }   // end of parallel

    // distribute evenly across segments and copy
    newDomain.finalize();
    newDomain.segment();
    // copy new domain into old csdomain
    cellSet->deepCopy(newCSDomain);
  }

public:
  csFromLevelSet() {}

  csFromLevelSet(LSType &passedLevelSet) : levelSet(&passedLevelSet) {}

  csFromLevelSet(LSType &passedLevelSet, CSType &passedCellSet)
      : levelSet(&passedLevelSet), cellSet(&passedCellSet) {}

  csFromLevelSet(LSType &passedLevelSet, CSType &passedCellSet, bool cff)
      : levelSet(&passedLevelSet), cellSet(&passedCellSet), calculateFillingFraction(cff) {}

  void setLevelSet(LSType &passedLevelSet) { levelSet = &passedLevelSet; }

  void setCellSet(CSType &passedCellSet) { cellSet = &passedCellSet; }

  void setCalculateFillingFraction(bool cff) { calculateFillingFraction = cff; }

  void apply() {
    if (levelSet == nullptr) {
      lsMessage::getInstance()
          .addWarning("No level set was passed to csFromLevelSet.")
          .print();
      return;
    }
    if (cellSet == nullptr) {
      lsMessage::getInstance()
          .addWarning("No cell set was passed to csFromLevelSet.")
          .print();
      return;
    }


    switch(conversionType) {
      case csFromLevelsetEnum::ANALYTICAL:
        // convertAnalytical
        break;
      case csFromLevelsetEnum::LOOKUP:
        // convertLookup
        break;
      case csFromLevelsetEnum::SIMPLE:
        convertSimple();
        break;
    }
  } // apply()
};

#endif // CS_FROM_LEVEL_SET_HPP
