/* -------------------------------------------------------------
   SPIDERHERO PROTOTYPE GAME ENGINE (THREE.JS / WEBGL)
   ------------------------------------------------------------- */
let scene, camera, renderer, clock;
let hero, heroMesh, heroLimbs = {};
let buildings = [], anchorPoints = [], enemies = [], webLines = [], trafficCars = [], civilians = [], rainParticles;
let currentWeather = 'clear';
let currentLighting = 'day';
let sunlight, ambientLight, fog;

// Physics & Hero State
const HeroState = {
    pos: new THREE.Vector3(0, 102, 0),
    vel: new THREE.Vector3(0, 0, 0),
    rot: 0,
    isGrounded: true,
    isSwinging: false,
    isZipping: false,
    isWallRunning: false,
    wallNormal: new THREE.Vector3(),
    wallRunSide: 'none',
    swingAnchor: null,
    swingRopeLength: 45,
    swingVelocity: new THREE.Vector3(),
    zipTarget: null,
    zipProgress: 0,
    health: 100,
    stamina: 100,
    xp: 0,
    level: 1,
    webAmmo: 8,
    mode: 'ROOFTOP',
    comboCount: 0,
    comboTimer: 0,
    spiderSenseTimer: 0,
    invulnerableTimer: 0
};

// Mission System State
let missionStep = 0;
const MissionSteps = [
    { text: "Hold SHIFT or RMB in mid-air to Web Swing across buildings", check: () => HeroState.isSwinging },
    { text: "Aim at a distant ledge and press E to Web Zip", check: () => HeroState.isZipping },
    { text: "Defeat the rooftop brawlers with LMB (Attack) and Q (Web Pull)", check: () => enemies.length === 0 },
    { text: "Rooftop Liberated! Free roam and explore the city district", check: () => false }
];

// Input Tracking
const Keys = {};
let mousePitch = 0.1, mouseYaw = 0;
let isPointerLocked = false;

// Procedural Audio Synthesizer (Web Audio API)
let audioCtx;
function initAudio() {
    if (!audioCtx) {
        audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    }
    if (audioCtx && audioCtx.state === 'suspended') {
        audioCtx.resume();
    }
}
function playThwipSound() {
    if (!audioCtx) return;
    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(800, audioCtx.currentTime);
    osc.frequency.exponentialRampToValueAtTime(120, audioCtx.currentTime + 0.15);
    gain.gain.setValueAtTime(0.3, audioCtx.currentTime);
    gain.gain.linearRampToValueAtTime(0.01, audioCtx.currentTime + 0.15);
    osc.connect(gain);
    gain.connect(audioCtx.destination);
    osc.start();
    osc.stop(audioCtx.currentTime + 0.16);
}
function playPunchSound() {
    if (!audioCtx) return;
    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();
    osc.type = 'triangle';
    osc.frequency.setValueAtTime(140, audioCtx.currentTime);
    osc.frequency.exponentialRampToValueAtTime(40, audioCtx.currentTime + 0.12);
    gain.gain.setValueAtTime(0.5, audioCtx.currentTime);
    gain.gain.linearRampToValueAtTime(0.01, audioCtx.currentTime + 0.12);
    osc.connect(gain);
    gain.connect(audioCtx.destination);
    osc.start();
    osc.stop(audioCtx.currentTime + 0.13);
}
function playSpiderSenseSound() {
    if (!audioCtx) return;
    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();
    osc.type = 'sawtooth';
    osc.frequency.setValueAtTime(950, audioCtx.currentTime);
    osc.frequency.linearRampToValueAtTime(1200, audioCtx.currentTime + 0.08);
    gain.gain.setValueAtTime(0.2, audioCtx.currentTime);
    gain.gain.linearRampToValueAtTime(0.01, audioCtx.currentTime + 0.15);
    osc.connect(gain);
    gain.connect(audioCtx.destination);
    osc.start();
    osc.stop(audioCtx.currentTime + 0.16);
}

// Init Game
function init() {
    const container = document.getElementById('canvas-container');
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x7ec0ee);
    fog = new THREE.FogExp2(0x7ec0ee, 0.0035);
    scene.fog = fog;

    camera = new THREE.PerspectiveCamera(85, window.innerWidth / window.innerHeight, 0.1, 1000);
    clock = new THREE.Clock();

    renderer = new THREE.WebGLRenderer({ antialias: true, powerPreference: "high-performance" });
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    container.appendChild(renderer.domElement);

    // Lighting
    ambientLight = new THREE.AmbientLight(0xffffff, 0.65);
    scene.add(ambientLight);

    sunlight = new THREE.DirectionalLight(0xfffaed, 1.2);
    sunlight.position.set(100, 250, 100);
    sunlight.castShadow = true;
    sunlight.shadow.mapSize.width = 2048;
    sunlight.shadow.mapSize.height = 2048;
    sunlight.shadow.camera.near = 10;
    sunlight.shadow.camera.far = 600;
    sunlight.shadow.camera.left = -200;
    sunlight.shadow.camera.right = 200;
    sunlight.shadow.camera.top = 200;
    sunlight.shadow.camera.bottom = -200;
    scene.add(sunlight);

    buildCity();
    createHero();
    createTraffic();
    createCivilians();
    createRainSystem();
    spawnEnemies();

    // Event Listeners
    window.addEventListener('resize', onWindowResize);
    document.addEventListener('keydown', onKeyDown);
    document.addEventListener('keyup', onKeyUp);
    document.addEventListener('mousedown', onMouseDown);
    document.addEventListener('mousemove', onMouseMove);

    renderer.domElement.addEventListener('click', () => {
        if (!isPointerLocked) {
            renderer.domElement.requestPointerLock();
        }
    });
    document.addEventListener('pointerlockchange', () => {
        isPointerLocked = document.pointerLockElement === renderer.domElement;
    });
}

// Build 3D City District
function buildCity() {
    const roadGeo = new THREE.PlaneGeometry(800, 800);
    const roadMat = new THREE.MeshStandardMaterial({ color: 0x181a1f, roughness: 0.85 });
    const road = new THREE.Mesh(roadGeo, roadMat);
    road.rotation.x = -Math.PI / 2;
    road.receiveShadow = true;
    scene.add(road);

    const gridHelper = new THREE.GridHelper(800, 40, 0xffeb3b, 0x334155);
    gridHelper.position.y = 0.1;
    scene.add(gridHelper);

    const colors = [0x1e293b, 0x0f172a, 0x334155, 0x1e1b4b, 0x172554, 0x3b82f6];

    for (let bx = -3; bx <= 3; bx++) {
        for (let bz = -3; bz <= 3; bz++) {
            if (bx === 0 && bz === 0) {
                createSkyscraper(0, 0, 40, 40, 100, 0x1e293b, true);
                continue;
            }
            const w = 25 + Math.random() * 20;
            const d = 25 + Math.random() * 20;
            const h = 50 + Math.random() * 110;
            const x = bx * 90 + (Math.random() - 0.5) * 15;
            const z = bz * 90 + (Math.random() - 0.5) * 15;
            const col = colors[Math.floor(Math.random() * colors.length)];
            createSkyscraper(x, z, w, d, h, col, false);
        }
    }
}

function createSkyscraper(x, z, w, d, h, col, isSpawn) {
    const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(w, h, d),
        new THREE.MeshStandardMaterial({
            color: col,
            roughness: 0.3,
            metalness: 0.4
        })
    );
    mesh.position.set(x, h / 2, z);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    scene.add(mesh);

    buildings.push({
        x, z, w, d, h,
        box: new THREE.Box3().setFromObject(mesh)
    });

    anchorPoints.push(
        new THREE.Vector3(x - w / 2, h, z - d / 2),
        new THREE.Vector3(x + w / 2, h, z - d / 2),
        new THREE.Vector3(x - w / 2, h, z + d / 2),
        new THREE.Vector3(x + w / 2, h, z + d / 2)
    );

    if (Math.random() > 0.4 || isSpawn) {
        const waterTower = new THREE.Mesh(
            new THREE.CylinderGeometry(4, 4, 8, 12),
            new THREE.MeshStandardMaterial({ color: 0x854d0e, roughness: 0.9 })
        );
        waterTower.position.set(x + (Math.random() - 0.5) * (w - 10), h + 4, z + (Math.random() - 0.5) * (d - 10));
        waterTower.castShadow = true;
        scene.add(waterTower);
        anchorPoints.push(new THREE.Vector3(waterTower.position.x, h + 8, waterTower.position.z));
    }
    if (Math.random() > 0.5) {
        const antenna = new THREE.Mesh(
            new THREE.CylinderGeometry(0.3, 0.8, 18, 8),
            new THREE.MeshStandardMaterial({ color: 0x94a3b8, metalness: 0.8 })
        );
        antenna.position.set(x, h + 9, z);
        scene.add(antenna);
        anchorPoints.push(new THREE.Vector3(x, h + 18, z));
    }
}

function createHero() {
    hero = new THREE.Group();
    
    const torsoMat = new THREE.MeshStandardMaterial({ color: 0xb91c1c, roughness: 0.35, metalness: 0.2 });
    const suitBlueMat = new THREE.MeshStandardMaterial({ color: 0x1d4ed8, roughness: 0.35, metalness: 0.2 });
    
    const torso = new THREE.Mesh(new THREE.BoxGeometry(0.9, 1.2, 0.5), torsoMat);
    torso.position.y = 1.6;
    torso.castShadow = true;
    hero.add(torso);

    const emblem = new THREE.Mesh(new THREE.OctahedronGeometry(0.2), new THREE.MeshBasicMaterial({ color: 0xffffff }));
    emblem.position.set(0, 1.7, 0.28);
    hero.add(emblem);

    const head = new THREE.Mesh(new THREE.SphereGeometry(0.35, 16, 16), torsoMat);
    head.position.y = 2.45;
    head.castShadow = true;
    hero.add(head);

    const eyeMat = new THREE.MeshBasicMaterial({ color: 0xffffff });
    const eyeL = new THREE.Mesh(new THREE.BoxGeometry(0.12, 0.08, 0.05), eyeMat);
    eyeL.position.set(-0.12, 2.48, 0.32);
    eyeL.rotation.z = -0.2;
    hero.add(eyeL);

    const eyeR = new THREE.Mesh(new THREE.BoxGeometry(0.12, 0.08, 0.05), eyeMat);
    eyeR.position.set(0.12, 2.48, 0.32);
    eyeR.rotation.z = 0.2;
    hero.add(eyeR);

    const armL = new THREE.Mesh(new THREE.BoxGeometry(0.3, 0.9, 0.3), torsoMat);
    armL.position.set(-0.65, 1.6, 0);
    armL.castShadow = true;
    hero.add(armL);
    heroLimbs.armL = armL;

    const armR = new THREE.Mesh(new THREE.BoxGeometry(0.3, 0.9, 0.3), torsoMat);
    armR.position.set(0.65, 1.6, 0);
    armR.castShadow = true;
    hero.add(armR);
    heroLimbs.armR = armR;

    const legL = new THREE.Mesh(new THREE.BoxGeometry(0.35, 1.0, 0.35), suitBlueMat);
    legL.position.set(-0.25, 0.6, 0);
    legL.castShadow = true;
    hero.add(legL);
    heroLimbs.legL = legL;

    const legR = new THREE.Mesh(new THREE.BoxGeometry(0.35, 1.0, 0.35), suitBlueMat);
    legR.position.set(0.25, 0.6, 0);
    legR.castShadow = true;
    hero.add(legR);
    heroLimbs.legR = legR;

    hero.position.copy(HeroState.pos);
    scene.add(hero);
}

function createTraffic() {
    const carColors = [0xfacc15, 0xef4444, 0x3b82f6, 0xffffff, 0x10b981];
    for (let i = 0; i < 30; i++) {
        const car = new THREE.Group();
        const col = carColors[Math.floor(Math.random() * carColors.length)];
        const body = new THREE.Mesh(new THREE.BoxGeometry(2.4, 1.2, 4.5), new THREE.MeshStandardMaterial({ color: col }));
        body.position.y = 0.8;
        body.castShadow = true;
        car.add(body);

        const lightMat = new THREE.MeshBasicMaterial({ color: 0xfffbeb });
        const hl1 = new THREE.Mesh(new THREE.SphereGeometry(0.2), lightMat);
        hl1.position.set(-0.8, 0.8, 2.3);
        car.add(hl1);
        const hl2 = new THREE.Mesh(new THREE.SphereGeometry(0.2), lightMat);
        hl2.position.set(0.8, 0.8, 2.3);
        car.add(hl2);

        const axis = Math.random() > 0.5 ? 'x' : 'z';
        const lane = (Math.floor(Math.random() * 7) - 3) * 90 + 45;
        const pos = (Math.random() - 0.5) * 600;
        
        if (axis === 'x') {
            car.position.set(pos, 0, lane);
            car.rotation.y = Math.PI / 2;
        } else {
            car.position.set(lane, 0, pos);
        }

        car.userData = { axis, speed: 18 + Math.random() * 12 };
        scene.add(car);
        trafficCars.push(car);
    }
}

function createCivilians() {
    for (let i = 0; i < 40; i++) {
        const civ = new THREE.Mesh(
            new THREE.CylinderGeometry(0.3, 0.3, 1.7, 8),
            new THREE.MeshStandardMaterial({ color: 0x64748b })
        );
        const x = (Math.floor(Math.random() * 7) - 3) * 90 + (Math.random() - 0.5) * 30;
        const z = (Math.floor(Math.random() * 7) - 3) * 90 + (Math.random() - 0.5) * 30;
        civ.position.set(x, 0.85, z);
        civ.castShadow = true;
        civ.userData = { speed: 1.5 + Math.random(), dir: Math.random() * Math.PI * 2 };
        scene.add(civ);
        civilians.push(civ);
    }
}

function createRainSystem() {
    const count = 1800;
    const geo = new THREE.BufferGeometry();
    const pos = [];
    for (let i = 0; i < count; i++) {
        pos.push((Math.random() - 0.5) * 250, Math.random() * 150, (Math.random() - 0.5) * 250);
    }
    geo.setAttribute('position', new THREE.Float32BufferAttribute(pos, 3));
    const mat = new THREE.PointsMaterial({ color: 0x93c5fd, size: 0.3, transparent: true, opacity: 0.6 });
    rainParticles = new THREE.Points(geo, mat);
    rainParticles.visible = false;
    scene.add(rainParticles);
}

function spawnEnemies() {
    enemies.forEach(e => scene.remove(e.mesh));
    enemies = [];

    const spawnPositions = [
        new THREE.Vector3(10, 101, 8),
        new THREE.Vector3(-12, 101, 10),
        new THREE.Vector3(8, 101, -12)
    ];

    spawnPositions.forEach((pos, idx) => {
        const enemyMesh = new THREE.Group();
        const brawlerMat = new THREE.MeshStandardMaterial({ color: 0x374151, roughness: 0.7 });
        const vestMat = new THREE.MeshStandardMaterial({ color: 0x991b1b });

        const body = new THREE.Mesh(new THREE.BoxGeometry(1.0, 1.3, 0.6), vestMat);
        body.position.y = 1.6;
        body.castShadow = true;
        enemyMesh.add(body);

        const head = new THREE.Mesh(new THREE.SphereGeometry(0.35, 12, 12), brawlerMat);
        head.position.y = 2.45;
        enemyMesh.add(head);

        const healthBarMesh = new THREE.Mesh(
            new THREE.PlaneGeometry(1.2, 0.15),
            new THREE.MeshBasicMaterial({ color: 0xef4444, side: THREE.DoubleSide })
        );
        healthBarMesh.position.y = 3.1;
        enemyMesh.add(healthBarMesh);

        enemyMesh.position.copy(pos);
        scene.add(enemyMesh);

        enemies.push({
            mesh: enemyMesh,
            pos: pos.clone(),
            vel: new THREE.Vector3(),
            health: 100,
            state: 'idle',
            attackCooldown: 2 + idx * 1.5,
            isWebbed: false,
            healthBar: healthBarMesh
        });
    });
    showNotification("ENEMY BRAWLERS SPAWNED ON ROOFTOP!");
}

function setLighting(mode) {
    currentLighting = mode;
    document.querySelectorAll('.env-btn').forEach(b => b.classList.remove('active'));
    const btn = document.getElementById('btn-' + mode);
    if (btn) btn.classList.add('active');

    if (mode === 'day') {
        scene.background.set(0x7ec0ee);
        fog.color.set(0x7ec0ee);
        sunlight.color.set(0xfffaed);
        sunlight.intensity = 1.2;
        ambientLight.intensity = 0.65;
    } else if (mode === 'sunset') {
        scene.background.set(0xf97316);
        fog.color.set(0xf97316);
        sunlight.color.set(0xfb923c);
        sunlight.intensity = 1.5;
        ambientLight.intensity = 0.45;
    } else if (mode === 'night') {
        scene.background.set(0x050811);
        fog.color.set(0x050811);
        sunlight.color.set(0x38bdf8);
        sunlight.intensity = 0.3;
        ambientLight.intensity = 0.2;
    }
}

function toggleWeather() {
    currentWeather = (currentWeather === 'clear') ? 'rain' : 'clear';
    rainParticles.visible = (currentWeather === 'rain');
    showNotification(currentWeather === 'rain' ? '🌧️ HEAVY RAIN ACTIVE' : '☀️ CLEAR SKIES');
}

function resetHero() {
    HeroState.pos.set(0, 102, 0);
    HeroState.vel.set(0, 0, 0);
    HeroState.isSwinging = false;
    HeroState.isZipping = false;
    HeroState.health = 100;
    HeroState.stamina = 100;
    updateHUD();
    showNotification('HERO RESET TO STARTING ROOFTOP');
}

function showNotification(text) {
    const notif = document.getElementById('notification');
    notif.innerText = text;
    notif.classList.add('show');
    setTimeout(() => notif.classList.remove('show'), 2200);
}

function onKeyDown(e) {
    Keys[e.code] = true;
    if (e.code === 'KeyE') tryWebZip();
    if (e.code === 'KeyQ') executeWebPull();
    if (e.code === 'KeyF') fireWebDart();
    if (e.code === 'KeyT') {
        const modes = ['day', 'sunset', 'night'];
        const next = modes[(modes.indexOf(currentLighting) + 1) % modes.length];
        setLighting(next);
    }
    if (e.code === 'KeyR') toggleWeather();
}

function onKeyUp(e) {
    Keys[e.code] = false;
    if (e.code === 'ShiftLeft' && HeroState.isSwinging) {
        releaseWebSwing();
    }
}

function onMouseDown(e) {
    initAudio();
    if (e.button === 0) {
        executeCombatAttack();
    } else if (e.button === 2) {
        if (!HeroState.isSwinging && !HeroState.isGrounded) {
            tryStartWebSwing();
        } else if (HeroState.isSwinging) {
            releaseWebSwing();
        }
    }
}

function onMouseMove(e) {
    if (!isPointerLocked) return;
    const sens = 0.0022;
    mouseYaw -= e.movementX * sens;
    mousePitch = Math.max(-1.1, Math.min(1.1, mousePitch - e.movementY * sens));
}

function findBestWebAnchor() {
    const camDir = new THREE.Vector3();
    camera.getWorldDirection(camDir);
    
    let bestPoint = null;
    let bestScore = -999;

    anchorPoints.forEach(pt => {
        const toPt = pt.clone().sub(camera.position);
        const dist = toPt.length();
        if (dist > 15 && dist < 160) {
            toPt.normalize();
            const dot = camDir.dot(toPt);
            if (dot > 0.75) {
                const heightAdvantage = (pt.y > HeroState.pos.y) ? 20 : -10;
                const score = (dot * 100) - (dist * 0.2) + heightAdvantage;
                if (score > bestScore) {
                    bestScore = score;
                    bestPoint = pt;
                }
            }
        }
    });
    return bestPoint;
}

function tryStartWebSwing() {
    const anchor = findBestWebAnchor();
    if (anchor && HeroState.stamina >= 10) {
        HeroState.isSwinging = true;
        HeroState.swingAnchor = anchor.clone();
        HeroState.swingRopeLength = HeroState.pos.distanceTo(anchor);
        HeroState.mode = 'WEB SWINGING';
        HeroState.stamina -= 8;
        playThwipSound();
        createWebLine(HeroState.pos, anchor);
        showNotification('🕸️ WEB ATTACHED • SWINGING');
    }
}

function releaseWebSwing() {
    if (!HeroState.isSwinging) return;
    HeroState.isSwinging = false;
    clearWebLines();
    
    const speed = HeroState.vel.length();
    if (speed > 8) {
        HeroState.vel.multiplyScalar(1.35);
        HeroState.vel.y = Math.max(HeroState.vel.y, 14);
        showNotification('🚀 APEX LAUNCH BOOST!');
    }
    HeroState.mode = 'AIRBORNE';
    addXP(25);
}

function tryWebZip() {
    initAudio();
    const anchor = findBestWebAnchor();
    if (anchor && !HeroState.isZipping) {
        HeroState.isZipping = true;
        HeroState.isSwinging = false;
        clearWebLines();
        HeroState.zipTarget = anchor.clone().add(new THREE.Vector3(0, 2, 0));
        HeroState.zipProgress = 0;
        HeroState.mode = 'WEB ZIP';
        playThwipSound();
        createWebLine(HeroState.pos, anchor);
        showNotification('⚡ WEB ZIP TRANSIT');
    }
}

function executeCombatAttack() {
    playPunchSound();
    HeroState.comboCount++;
    HeroState.comboTimer = 2.5;

    heroLimbs.armR.rotation.x = -Math.PI / 2;
    setTimeout(() => { heroLimbs.armR.rotation.x = 0; }, 180);

    enemies.forEach(e => {
        const dist = e.mesh.position.distanceTo(HeroState.pos);
        if (dist < 4.5) {
            e.health -= 35;
            e.mesh.position.add(new THREE.Vector3(0, 2.5, 0).add(camera.getWorldDirection(new THREE.Vector3()).multiplyScalar(3)));
            e.healthBar.scale.x = Math.max(0, e.health / 100);
            showNotification(`💥 COMBO HIT! [${e.health > 0 ? e.health + ' HP' : 'KNOCKED OUT'}]`);
            addXP(50);
            if (e.health <= 0) {
                scene.remove(e.mesh);
                enemies = enemies.filter(item => item !== e);
            }
        }
    });
    updateHUD();
}

function executeWebPull() {
    initAudio();
    playThwipSound();
    enemies.forEach(e => {
        const dist = e.mesh.position.distanceTo(HeroState.pos);
        if (dist < 35) {
            createWebLine(HeroState.pos, e.mesh.position);
            e.mesh.position.lerp(HeroState.pos, 0.7);
            showNotification('🕸️ ENEMY YANKED & BOUND!');
            addXP(75);
        }
    });
    setTimeout(clearWebLines, 250);
}

function fireWebDart() {
    initAudio();
    if (HeroState.webAmmo <= 0) return;
    HeroState.webAmmo--;
    playThwipSound();
    updateHUD();
}

function addXP(amount) {
    HeroState.xp += amount;
    if (HeroState.xp >= HeroState.level * 250) {
        HeroState.level++;
        showNotification(`🌟 LEVEL UP! YOU ARE NOW LEVEL ${HeroState.level}`);
    }
    updateHUD();
}

function createWebLine(start, end) {
    clearWebLines();
    const geo = new THREE.BufferGeometry().setFromPoints([start, end]);
    const mat = new THREE.LineBasicMaterial({ color: 0xffffff, linewidth: 3 });
    const line = new THREE.Line(geo, mat);
    scene.add(line);
    webLines.push(line);
}

function clearWebLines() {
    webLines.forEach(l => scene.remove(l));
    webLines = [];
}

function updatePhysics(dt) {
    const moveDir = new THREE.Vector3();
    if (Keys['KeyW']) moveDir.z -= 1;
    if (Keys['KeyS']) moveDir.z += 1;
    if (Keys['KeyA']) moveDir.x -= 1;
    if (Keys['KeyD']) moveDir.x += 1;
    moveDir.applyAxisAngle(new THREE.Vector3(0, 1, 0), mouseYaw);
    moveDir.normalize();

    let onWall = false;
    buildings.forEach(b => {
        if (Math.abs(HeroState.pos.x - b.x) < (b.w / 2 + 1.2) && Math.abs(HeroState.pos.z - b.z) < (b.d / 2 + 1.2) && HeroState.pos.y < b.h && HeroState.pos.y > 0) {
            onWall = true;
            HeroState.isWallRunning = true;
            HeroState.mode = 'WALL RUNNING';
        }
    });

    if (HeroState.isZipping) {
        HeroState.zipProgress += dt * 3.5;
        HeroState.pos.lerp(HeroState.zipTarget, 0.15);
        if (HeroState.zipProgress >= 1 || HeroState.pos.distanceTo(HeroState.zipTarget) < 2) {
            HeroState.isZipping = false;
            clearWebLines();
            HeroState.vel.set(0, 0, 0);
            if (Keys['Space']) {
                HeroState.vel.y = 22;
                showNotification('🚀 POINT LAUNCH BOOST!');
            }
        }
    } else if (HeroState.isSwinging && HeroState.swingAnchor) {
        const toAnchor = HeroState.swingAnchor.clone().sub(HeroState.pos);
        const currentDist = toAnchor.length();
        const ropeDir = toAnchor.clone().normalize();

        HeroState.vel.y -= 28 * dt;
        
        if (moveDir.length() > 0) {
            HeroState.vel.add(moveDir.clone().multiplyScalar(35 * dt));
        }

        if (currentDist >= HeroState.swingRopeLength) {
            const radialSpeed = HeroState.vel.dot(ropeDir);
            if (radialSpeed < 0) {
                HeroState.vel.sub(ropeDir.clone().multiplyScalar(radialSpeed * 1.05));
            }
            HeroState.pos.copy(HeroState.swingAnchor.clone().sub(ropeDir.multiplyScalar(HeroState.swingRopeLength)));
        }

        HeroState.pos.add(HeroState.vel.clone().multiplyScalar(dt));
        if (webLines[0]) {
            webLines[0].geometry.setFromPoints([HeroState.pos.clone().add(new THREE.Vector3(0, 1.6, 0)), HeroState.swingAnchor]);
        }
    } else {
        const speed = (Keys['ShiftLeft'] || Keys['ShiftRight']) ? 22 : 12;
        
        if (HeroState.isGrounded) {
            HeroState.vel.x = moveDir.x * speed;
            HeroState.vel.z = moveDir.z * speed;
            if (Keys['Space']) {
                HeroState.vel.y = 16;
                HeroState.isGrounded = false;
                HeroState.stamina -= 5;
            }
        } else {
            const grav = (Math.abs(HeroState.vel.y) < 3) ? 14 : 26;
            HeroState.vel.y -= grav * dt;
            HeroState.vel.x += moveDir.x * speed * dt * 2.5;
            HeroState.vel.z += moveDir.z * speed * dt * 2.5;
            HeroState.vel.x *= 0.98;
            HeroState.vel.z *= 0.98;
        }

        HeroState.pos.add(HeroState.vel.clone().multiplyScalar(dt));

        let groundedThisFrame = false;
        buildings.forEach(b => {
            if (HeroState.pos.x > b.x - b.w / 2 && HeroState.pos.x < b.x + b.w / 2 &&
                HeroState.pos.z > b.z - b.d / 2 && HeroState.pos.z < b.z + b.d / 2) {
                if (HeroState.pos.y <= b.h + 0.1 && HeroState.pos.y >= b.h - 3) {
                    HeroState.pos.y = b.h;
                    HeroState.vel.y = 0;
                    groundedThisFrame = true;
                    HeroState.mode = 'ROOFTOP';
                }
            }
        });

        if (HeroState.pos.y <= 0) {
            HeroState.pos.y = 0;
            HeroState.vel.y = 0;
            groundedThisFrame = true;
            HeroState.mode = 'STREET LEVEL';
        }

        HeroState.isGrounded = groundedThisFrame;
        if (!HeroState.isGrounded && !HeroState.isSwinging && !HeroState.isZipping) {
            HeroState.mode = 'AIRBORNE';
        }
    }

    hero.position.copy(HeroState.pos);
    if (moveDir.length() > 0.1) {
        const targetRot = Math.atan2(moveDir.x, moveDir.z);
        hero.rotation.y = targetRot;
    }

    const camOffset = new THREE.Vector3(0, 3.2, 7.5);
    camOffset.applyAxisAngle(new THREE.Vector3(1, 0, 0), mousePitch);
    camOffset.applyAxisAngle(new THREE.Vector3(0, 1, 0), mouseYaw);
    
    const targetCamPos = HeroState.pos.clone().add(camOffset);
    camera.position.lerp(targetCamPos, 0.12);
    camera.lookAt(HeroState.pos.clone().add(new THREE.Vector3(0, 1.8, 0)));

    const currentSpeed = HeroState.vel.length();
    const targetFOV = HeroState.isSwinging ? 108 : (Keys['ShiftLeft'] ? 96 : 85);
    camera.fov = THREE.MathUtils.lerp(camera.fov, targetFOV, 0.08);
    camera.updateProjectionMatrix();

    const speedLines = document.getElementById('speed-lines');
    if (speedLines) speedLines.style.opacity = currentSpeed > 18 ? '0.7' : '0';

    const target = findBestWebAnchor();
    const reticle = document.getElementById('reticle');
    if (reticle) {
        if (target) {
            reticle.classList.add('locked');
        } else {
            reticle.classList.remove('locked');
        }
    }

    trafficCars.forEach(car => {
        if (car.userData.axis === 'x') {
            car.position.x += car.userData.speed * dt;
            if (car.position.x > 350) car.position.x = -350;
        } else {
            car.position.z += car.userData.speed * dt;
            if (car.position.z > 350) car.position.z = -350;
        }
    });

    HeroState.stamina = Math.min(100, HeroState.stamina + dt * 12);
    
    if (HeroState.comboTimer > 0) {
        HeroState.comboTimer -= dt;
        if (HeroState.comboTimer <= 0) {
            HeroState.comboCount = 0;
            const box = document.getElementById('combo-box');
            if (box) box.classList.remove('active');
        }
    }

    if (missionStep < MissionSteps.length && MissionSteps[missionStep].check()) {
        const checkEl = document.getElementById('step-check');
        if (checkEl) {
            checkEl.classList.add('done');
            checkEl.innerText = '✓';
        }
        setTimeout(() => {
            missionStep++;
            if (missionStep < MissionSteps.length) {
                if (checkEl) {
                    checkEl.classList.remove('done');
                    checkEl.innerText = '';
                }
                const stepText = document.getElementById('step-text');
                if (stepText) stepText.innerText = MissionSteps[missionStep].text;
                showNotification('🎯 OBJECTIVE COMPLETED! +100 XP');
                addXP(100);
            }
        }, 800);
    }

    updateHUD();
}

function updateHUD() {
    const healthEl = document.getElementById('ui-health');
    const staminaEl = document.getElementById('ui-stamina');
    const xpEl = document.getElementById('ui-xp');
    const levelEl = document.getElementById('ui-level');
    const stateEl = document.getElementById('ui-state');

    if (healthEl) healthEl.style.width = HeroState.health + '%';
    if (staminaEl) staminaEl.style.width = HeroState.stamina + '%';
    if (xpEl) xpEl.style.width = (HeroState.xp % 250) / 2.5 + '%';
    if (levelEl) levelEl.innerText = `LEVEL ${HeroState.level} • ${HeroState.xp} XP`;
    if (stateEl) stateEl.innerText = HeroState.mode;

    if (HeroState.comboCount > 0) {
        const box = document.getElementById('combo-box');
        if (box) {
            box.classList.add('active');
            const numEl = document.getElementById('combo-num');
            const multEl = document.getElementById('combo-mult');
            if (numEl) numEl.innerText = HeroState.comboCount;
            if (multEl) multEl.innerText = (1.0 + HeroState.comboCount * 0.1).toFixed(1) + 'x';
        }
    }
}

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function startGame() {
    initAudio();
    const overlay = document.getElementById('cinematic-overlay');
    if (overlay) overlay.classList.add('fade-out');
    renderer.domElement.requestPointerLock();
    showNotification('⚡ SPIDER-HERO PROTOTYPE ACTIVE');
}

function animate() {
    requestAnimationFrame(animate);
    const dt = Math.min(clock.getDelta(), 0.1);
    updatePhysics(dt);
    renderer.render(scene, camera);
}

window.onload = () => {
    init();
    animate();
};
