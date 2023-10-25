# switch between 2D and 3D mode
DIM = 2

if DIM == 2:
    import viennaps2d as vps
else:
    import viennaps3d as vps

params = vps.ReadConfigFile("SingleTEOS_config.txt")

geometry = vps.Domain()
vps.MakeTrench(
    domain=geometry,
    gridDelta=params["gridDelta"],
    xExtent=params["xExtent"],
    yExtent=params["yExtent"],
    trenchWidth=params["trenchWidth"],
    trenchDepth=params["trenchHeight"],
    taperingAngle=params["taperAngle"],
    baseHeight=0.0,
    periodicBoundary=False,
    makeMask=False,
    material=vps.Material.Si,
).apply()

# copy top layer to capture deposition
geometry.duplicateTopLevelSet(vps.Material.SiO2)

# process model encompasses surface model and particle types
model = vps.TEOSDeposition(
    stickingProbabilityP1=params["stickingProbabilityP1"],
    rateP1=params["depositionRateP1"],
    orderP1=params["reactionOrderP1"],
)

process = vps.Process()
process.setDomain(geometry)
process.setProcessModel(model)
process.setNumberOfRaysPerPoint(int(params["numRaysPerPoint"]))
process.setProcessDuration(params["processTime"])

geometry.printSurface("SingleTEOS_initial.vtp")

process.apply()

geometry.printSurface("SingleTEOS_final.vtp")

if DIM == 2:
    vps.WriteVisualizationMesh(geometry, "SingleTEOS_final").apply()