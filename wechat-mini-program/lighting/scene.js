const { createGestures } = require('./gestures')
const { sampleEffect } = require('./effects')
const physical = require('../painting/physical-map')
const physicalIndex = new Map(physical.points.map((p,i)=>[`${p.x.toFixed(4)},${p.y.toFixed(4)}`,i]))

function createScene(canvas, THREE, points, options) {
  const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: false })
  const scene = new THREE.Scene()
  const camera = new THREE.PerspectiveCamera(36, 1, 1, 3000)
  const gestures = createGestures()
  const geometries = new Set(), materials = new Set()
  let disposed = false
  let viewport={width:options.width,height:options.height}
  const ownGeometry = g => { geometries.add(g); return g }
  const ownMaterial = m => { materials.add(m); return m }
  const ratio = Math.max(1, Math.min(2, Number.isFinite(options.pixelRatio) ? options.pixelRatio : 1))
  try {
    renderer.setPixelRatio(ratio)
    renderer.setClearColor(0x20242d, 1)
    const boxGeometry = ownGeometry(new THREE.BoxBufferGeometry(200, 300, 200))
    const glass = new THREE.Mesh(boxGeometry, ownMaterial(new THREE.MeshBasicMaterial({
      color: 0xa0b1c6, transparent: true, opacity: 0.018, depthWrite: false, side: THREE.DoubleSide
    })))
    scene.add(glass)
    scene.add(new THREE.LineSegments(ownGeometry(new THREE.EdgesGeometry(boxGeometry)),
      ownMaterial(new THREE.LineBasicMaterial({ color: 0x9caec6, transparent: true, opacity: 0.3 }))))
    const pedestal = new THREE.Mesh(ownGeometry(new THREE.CylinderBufferGeometry(43, 48, 10, 32)),
      ownMaterial(new THREE.MeshBasicMaterial({ color: 0x354050 })))
    pedestal.position.y = -140; scene.add(pedestal)
    const figureMaterial = ownMaterial(new THREE.MeshBasicMaterial({ color: 0x9ba7b9, wireframe: true, transparent: true, opacity: 0.16 }))
    const body = new THREE.Mesh(ownGeometry(new THREE.CylinderBufferGeometry(17, 27, 100, 12)), figureMaterial)
    body.position.y = -80; scene.add(body)
    const head = new THREE.Mesh(ownGeometry(new THREE.SphereBufferGeometry(20, 12, 8)), figureMaterial)
    head.position.y = -10; scene.add(head)
    const positions = new Float32Array(points.length * 3), colors = new Float32Array(points.length * 3)
    points.forEach((p, i) => positions.set([p.x, p.y, p.z], i * 3))
    const geometry = ownGeometry(new THREE.BufferGeometry())
    geometry.addAttribute('position', new THREE.BufferAttribute(positions, 3))
    geometry.addAttribute('color', new THREE.BufferAttribute(colors, 3))
    const ledMaterial = ownMaterial(new THREE.ShaderMaterial({
      transparent: true, depthWrite: false, depthTest: false, blending: THREE.AdditiveBlending,
      uniforms: { pointSize: { value: 26 * ratio } },
      vertexShader: 'attribute vec3 color; varying vec3 ledColor; uniform float pointSize; void main(){ledColor=color; gl_Position=projectionMatrix*modelViewMatrix*vec4(position,1.0); gl_PointSize=pointSize;}',
      // Additive light cannot darken the background. Zero-output LEDs retain
      // only a tiny locator; preview gain never changes BLE/firmware values.
      fragmentShader: `varying vec3 ledColor;
        void main(){
          float r=length(gl_PointCoord-vec2(0.5));
          if(r>0.5) discard;
          float peak=max(ledColor.r,max(ledColor.g,ledColor.b));
          float gain=pow(max(peak,0.0),0.65);
          vec3 tint=ledColor/max(peak,0.001);
          float core=1.0-smoothstep(0.035,0.10,r);
          float halo=exp(-r*r*24.0)*(1.0-smoothstep(0.32,0.5,r));
          float locator=(1.0-smoothstep(0.025,0.055,r))*0.035;
          vec3 light=tint*gain*(halo*0.85+core*0.65)+vec3(gain*core*0.65);
          gl_FragColor=vec4(light+vec3(locator),1.0);
        }`
    }))
    const ledPoints=new THREE.Points(geometry,ledMaterial)
    ledPoints.renderOrder=10
    scene.add(ledPoints)
    function updateCamera() {
      const { yaw, pitch, distance } = gestures.state
      camera.position.set(Math.sin(pitch) * Math.sin(yaw) * distance,
        Math.cos(pitch) * distance, Math.sin(pitch) * Math.cos(yaw) * distance)
      camera.lookAt(0, 0, 0)
    }
    const api = {
      render(settings, time) {
        if (disposed) return
        points.forEach((p, i) => {
          let rgb = sampleEffect(p, settings, time)
          if(settings.paintFrame) {
            const index=physicalIndex.get(`${p.x.toFixed(4)},${(-p.z).toFixed(4)}`)
            rgb=index===undefined?[0,0,0]:[0,1,2].map(c=>settings.paintFrame[index*3+c]/255)
          }
          colors.set(rgb, i * 3)
        })
        geometry.attributes.color.needsUpdate = true
        updateCamera(); renderer.render(scene, camera)
      },
      resize(width, height) {
        if (disposed) return
        const w = Math.max(1, Number.isFinite(width) ? width : 1)
        const h = Math.max(1, Number.isFinite(height) ? height : 1)
        viewport={width:w,height:h}
        renderer.setSize(w, h, false); camera.aspect = w / h; camera.updateProjectionMatrix()
      },
      setView(name) { if (!disposed) gestures.setView(name) },
      touchStart(event) { if (!disposed) gestures.start(event.touches) },
      touchMove(event) { if (!disposed) gestures.move(event.touches) },
      touchEnd() { gestures.end() },
      hitBottom(x,y) {
        if(disposed||!Number.isFinite(x)||!Number.isFinite(y))return false
        updateCamera();camera.updateMatrixWorld()
        const raycaster=new THREE.Raycaster()
        raycaster.setFromCamera(new THREE.Vector2(x/viewport.width*2-1,1-y/viewport.height*2),camera)
        const ray=raycaster.ray
        if(ray.direction.y>=-0.0001)return false
        const t=(-150-ray.origin.y)/ray.direction.y
        const px=ray.origin.x+t*ray.direction.x,pz=ray.origin.z+t*ray.direction.z
        return t>0&&Math.abs(px)<=100&&Math.abs(pz)<=100
      },
      dispose() {
        if (disposed) return
        disposed = true; gestures.end()
        geometries.forEach(g => g.dispose()); materials.forEach(m => m.dispose())
        renderer.dispose()
      }
    }
    api.resize(options.width, options.height)
    return api
  } catch (error) {
    geometries.forEach(g => g.dispose()); materials.forEach(m => m.dispose()); renderer.dispose()
    throw error
  }
}
module.exports = { createScene }
