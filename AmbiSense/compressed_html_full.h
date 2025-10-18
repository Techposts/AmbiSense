#ifndef COMPRESSED_HTML_FULL_H
#define COMPRESSED_HTML_FULL_H

#include <pgmspace.h>

// Full-featured compressed HTML with all original styling and functionality
// This preserves the complete professional UI with live updates, sliders, and visual feedback

// Complete CSS (minified but preserving all features)
const char full_css[] PROGMEM = R"literal(
:root{--primary:#4361ee;--primary-light:#4895ef;--success:#4cc9f0;--bg-dark:#121212;--bg-card:#1e1e1e;--text:#ffffff;--text-secondary:#b0b0b0;--border-radius:12px;--shadow:0 10px 20px rgba(0,0,0,0.3);--transition:all 0.3s ease}
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background-color:var(--bg-dark);color:var(--text);min-height:100vh;display:flex;flex-direction:column;justify-content:space-between;padding:20px;background:linear-gradient(135deg,#121212 0%,#2a2a2a 100%)}
.main-content{display:flex;justify-content:center;align-items:center;flex:1}
.container{background:var(--bg-card);border-radius:var(--border-radius);box-shadow:var(--shadow);width:100%;max-width:420px;overflow:hidden;position:relative}
.header{background:linear-gradient(135deg,var(--primary) 0%,var(--primary-light) 100%);padding:25px 20px;text-align:center;position:relative;overflow:hidden}
.header h1{font-size:24px;font-weight:600;margin-bottom:8px;text-shadow:0 2px 5px rgba(0,0,0,0.2);position:relative;z-index:2}
.header p{opacity:0.9;font-size:14px;position:relative;z-index:2}
.header::before{content:'';position:absolute;top:-50%;left:-50%;width:200%;height:200%;background:radial-gradient(circle,rgba(255,255,255,0.1) 0%,rgba(255,255,255,0) 70%);z-index:1}
.content{padding:25px}
.form-group{margin-bottom:22px;position:relative}
.form-group label{display:block;margin-bottom:8px;font-size:14px;color:var(--text-secondary);font-weight:500}
.slider-container{display:flex;align-items:center;gap:15px}
.slider{flex:1;-webkit-appearance:none;height:8px;border-radius:5px;background:#333;outline:none;transition:var(--transition)}
.slider::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;width:18px;height:18px;border-radius:50%;background:var(--primary);cursor:pointer;transition:var(--transition);box-shadow:0 0 10px rgba(67,97,238,0.5)}
.slider::-moz-range-thumb{width:18px;height:18px;border-radius:50%;background:var(--primary);cursor:pointer;transition:var(--transition);box-shadow:0 0 10px rgba(67,97,238,0.5)}
.slider:hover::-webkit-slider-thumb{transform:scale(1.1)}
.slider-value{min-width:60px;padding:8px 12px;border-radius:var(--border-radius);background:#333;text-align:center;font-weight:600;font-size:14px;color:var(--text)}
.color-preview{width:60px;height:36px;border-radius:var(--border-radius);overflow:hidden;box-shadow:inset 0 0 5px rgba(0,0,0,0.5)}
.rgb-sliders{margin-top:15px}
.rgb-label{display:inline-block;width:20px;text-align:center;font-weight:bold;margin-right:10px}
.rgb-label.r{color:#ff5555}
.rgb-label.g{color:#55ff55}
.rgb-label.b{color:#5555ff}
.led-count-container{display:flex;align-items:center;gap:15px}
.led-count-input{flex:1;padding:10px 12px;border-radius:var(--border-radius);background:#333;border:none;color:var(--text);font-size:14px;text-align:center}
.button{display:block;width:100%;padding:15px;background:linear-gradient(135deg,var(--primary) 0%,var(--primary-light) 100%);color:white;border:none;border-radius:var(--border-radius);font-size:16px;font-weight:600;cursor:pointer;transition:var(--transition);margin-top:30px;text-transform:uppercase;letter-spacing:1px;box-shadow:0 5px 15px rgba(67,97,238,0.4)}
.button:hover{transform:translateY(-3px);box-shadow:0 8px 20px rgba(67,97,238,0.6)}
.settings-saved{position:fixed;top:20px;right:20px;padding:15px 25px;background:linear-gradient(135deg,#4cc9f0 0%,#4361ee 100%);color:white;border-radius:var(--border-radius);transform:translateX(200%);transition:transform 0.3s ease;box-shadow:0 5px 15px rgba(0,0,0,0.2);z-index:1000}
.settings-saved.show{transform:translateX(0)}
.footer{text-align:center;padding:15px 0;margin-top:30px;font-size:12px;color:var(--text-secondary);width:100%}
.footer p{margin:3px 0}
.branding{font-weight:600;color:var(--text)}
.loading-overlay{position:absolute;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.7);display:flex;justify-content:center;align-items:center;z-index:9999}
.spinner{width:50px;height:50px;border:5px solid rgba(0,0,0,0.1);border-radius:50%;border-top-color:var(--primary);animation:spin 1s ease-in-out infinite}
.tab-container{display:flex;margin-bottom:20px;border-bottom:1px solid #333;overflow-x:auto;white-space:nowrap}
.tab{flex:1 0 auto;padding:10px 8px;cursor:pointer;background-color:transparent;border:none;color:var(--text-secondary);font-size:12px;position:relative;text-overflow:ellipsis;overflow:hidden;max-width:100px}
.tab.active{color:var(--primary)}
.tab.active::after{content:'';position:absolute;bottom:-1px;left:0;width:100%;height:2px;background-color:var(--primary)}
@keyframes spin{to{transform:rotate(360deg)}}
.distance-display{margin-top:30px;text-align:center;padding:20px;border-radius:var(--border-radius);background:rgba(30,30,30,0.6);position:relative;overflow:hidden}
.distance-display::before{content:'';position:absolute;top:0;left:0;width:100%;height:2px;background:linear-gradient(90deg,var(--primary) 0%,var(--success) 100%)}
.distance-label{font-size:14px;margin-bottom:10px;color:var(--text-secondary)}
.distance-value{font-size:36px;font-weight:700;color:var(--success);text-shadow:0 0 10px rgba(76,201,240,0.5);transition:var(--transition)}
.distance-unit{font-size:16px;opacity:0.7;margin-left:5px}
.led-visualizer{height:15px;background:#333;margin-top:15px;border-radius:10px;overflow:hidden;position:relative}
.led-active{position:absolute;height:100%;border-radius:10px;transition:var(--transition)}
.led-preview-container{background:#2a2a2a;padding:20px;margin:20px 0;border-radius:10px;border-left:4px solid var(--primary)}
.led-preview-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:15px}
.led-preview-title{color:var(--primary);font-size:16px;font-weight:600}
.led-preview-controls{display:flex;gap:10px}
.led-preview-btn{padding:6px 12px;background:#4361ee;color:white;border:none;border-radius:6px;cursor:pointer;font-size:12px}
.led-preview-btn.active{background:var(--success)}
.led-strip-visual{height:40px;background:#1a1a1a;border-radius:8px;position:relative;overflow:hidden;margin:10px 0;border:1px solid #444}
.led-pixel{position:absolute;top:2px;bottom:2px;width:3px;margin:0 0.5px;border-radius:1px;transition:all 0.3s ease}
.led-info-bar{display:flex;justify-content:space-between;align-items:center;margin-top:10px;font-size:12px;color:var(--text-secondary)}
.led-count-display{color:var(--success);font-weight:600}
.led-position-display{color:var(--primary)}
.mesh-segment-preview{margin-top:15px;padding:15px;background:#333;border-radius:8px}
.segment-strip{height:30px;background:#1a1a1a;border-radius:6px;position:relative;margin:8px 0;border:1px solid #555}
.segment-label{font-size:11px;color:var(--text-secondary);margin-bottom:5px}
.segment-active{border-color:var(--success);box-shadow:0 0 8px rgba(76,201,240,0.3)}
.toggle-container{display:flex;justify-content:space-between;align-items:center;margin-bottom:20px}
.toggle-label{font-size:14px;color:var(--text-secondary)}
.toggle-switch{position:relative;display:inline-block;width:60px;height:34px}
.toggle-switch input{opacity:0;width:0;height:0}
.toggle-slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#333;transition:.4s;border-radius:34px}
.toggle-slider:before{position:absolute;content:"";height:26px;width:26px;left:4px;bottom:4px;background-color:white;transition:.4s;border-radius:50%}
input:checked+.toggle-slider{background-color:var(--primary)}
input:checked+.toggle-slider:before{transform:translateX(26px)}
.input-description{display:block;margin-top:5px;font-size:12px;color:var(--text-secondary)}
@media (max-width:480px){.container{max-width:100%;margin:0}body{padding:10px}.tab{font-size:10px;padding:8px 4px}}
)literal";

// Complete JavaScript functionality (minified but preserving all features)
const char full_js[] PROGMEM = R"literal(
function showSavedNotification(){if(window.savedNotificationTimeout){clearTimeout(window.savedNotificationTimeout)}const notification=document.getElementById('settingsSaved');notification.classList.remove('show');setTimeout(()=>{notification.classList.add('show');window.savedNotificationTimeout=setTimeout(()=>{notification.classList.remove('show')},3000)},10)}
function updateDistance(){fetch('/distance').then(response=>response.text()).then(distance=>{distance=parseInt(distance);document.getElementById('liveDistance').innerHTML=distance+'<span class="distance-unit">cm</span>';const minDist=parseInt(document.getElementById('minDist')?.value||'30');const maxDist=parseInt(document.getElementById('maxDist')?.value||'300');const percentage=Math.min(100,Math.max(0,((distance-minDist)/(maxDist-minDist))*100));const ledVisualizer=document.getElementById('ledVisualizer');const directionLightElement=document.getElementById('directionLight');if(directionLightElement&&directionLightElement.checked){const position=100-percentage;ledVisualizer.style.width='30%';ledVisualizer.style.left=`${position-15}%`}else{ledVisualizer.style.width=(100-percentage)+'%';ledVisualizer.style.left='0'}const redElement=document.getElementById('redValue');const greenElement=document.getElementById('greenValue');const blueElement=document.getElementById('blueValue');if(redElement&&greenElement&&blueElement){const red=parseInt(redElement.value);const green=parseInt(greenElement.value);const blue=parseInt(blueElement.value);ledVisualizer.style.background=`rgb(${red}, ${green}, ${blue})`}updateLEDPreview(distance,percentage)}).catch(error=>{console.error("Error fetching distance:",error)}).finally(()=>{setTimeout(updateDistance,500)})}
function updateColorPreview(){const redElement=document.getElementById('redValue');const greenElement=document.getElementById('greenValue');const blueElement=document.getElementById('blueValue');const colorPreview=document.getElementById('colorPreview');if(redElement&&greenElement&&blueElement&&colorPreview){const red=parseInt(redElement.value);const green=parseInt(greenElement.value);const blue=parseInt(blueElement.value);colorPreview.style.background=`rgb(${red}, ${green}, ${blue})`}}
function autoSaveChanges(settingId,value){let params='';if(settingId==='numLeds')params='numLeds='+value;else if(settingId==='minDist')params='minDist='+value;else if(settingId==='maxDist')params='maxDist='+value;else if(settingId==='brightness')params='brightness='+value;else if(settingId==='redValue')params='redValue='+value;else if(settingId==='greenValue')params='greenValue='+value;else if(settingId==='blueValue')params='blueValue='+value;else if(settingId==='lightSpan')params='lightSpan='+value;else return;const affectedControl=document.getElementById(settingId);const originalValue=affectedControl.value;fetch('/set?'+params).then(response=>{if(!response.ok){throw new Error('Server returned status: '+response.status)}return response.json()}).then(data=>{if(data.status==='success'){showSavedNotification();triggerLEDPreviewUpdate()}else if(data.status==='error'){affectedControl.value=originalValue;const displayElement=document.getElementById(settingId+(settingId.includes('Value')?'Display':'Value'));if(displayElement){displayElement.textContent=originalValue}alert('Error saving setting: '+(data.message||'Unknown error'))}}).catch(error=>{console.error('Error saving setting:',error);affectedControl.value=originalValue;const displayElement=document.getElementById(settingId+(settingId.includes('Value')?'Display':'Value'));if(displayElement){displayElement.textContent=originalValue}alert('Failed to save setting. Please try again.')})}
function setBackgroundMode(enabled){fetch('/setBackgroundMode?enabled='+enabled).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification();triggerLEDPreviewUpdate()}}).catch(error=>console.error('Error:',error))}
function setDirectionalLight(enabled){fetch('/setDirectionalLight?enabled='+enabled).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification();triggerLEDPreviewUpdate()}}).catch(error=>console.error('Error:',error))}
function setMotionSmoothing(enabled){fetch('/setMotionSmoothing?enabled='+enabled).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification();triggerLEDPreviewUpdate()}}).catch(error=>console.error('Error:',error))}
function setLightMode(mode){fetch('/setLightMode?mode='+mode).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification();triggerLEDPreviewUpdate()}}).catch(error=>console.error('Error:',error))}
function updateMotionParam(param,value){fetch('/setMotionSmoothingParam?param='+param+'&value='+value).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>console.error('Error:',error))}
function setEffectSpeed(value){fetch('/setEffectSpeed?value='+value).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>console.error('Error:',error))}
function setEffectIntensity(value){fetch('/setEffectIntensity?value='+value).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>console.error('Error:',error))}
function saveAdvancedSettings(){const settings={lightSpan:document.getElementById('lightSpan').value,trailLength:document.getElementById('trailLength').value,centerShift:document.getElementById('centerShift').value,positionSmoothing:document.getElementById('positionSmoothing').value,velocitySmoothing:document.getElementById('velocitySmoothing').value,predictionFactor:document.getElementById('predictionFactor').value,positionPGain:document.getElementById('positionPGain').value,positionIGain:document.getElementById('positionIGain').value,backgroundMode:document.getElementById('backgroundMode').checked,directionLight:document.getElementById('directionLight').checked,motionSmoothing:document.getElementById('motionSmoothing').checked};const promises=[];promises.push(fetch('/set?lightSpan='+settings.lightSpan));promises.push(fetch('/setTrailLength?value='+settings.trailLength));promises.push(fetch('/setCenterShift?value='+settings.centerShift));promises.push(fetch('/setMotionSmoothingParam?param=positionSmoothingFactor&value='+(settings.positionSmoothing/100)));promises.push(fetch('/setMotionSmoothingParam?param=velocitySmoothingFactor&value='+(settings.velocitySmoothing/100)));promises.push(fetch('/setMotionSmoothingParam?param=predictionFactor&value='+(settings.predictionFactor/100)));promises.push(fetch('/setMotionSmoothingParam?param=positionPGain&value='+(settings.positionPGain/100)));promises.push(fetch('/setMotionSmoothingParam?param=positionIGain&value='+(settings.positionIGain/100)));promises.push(fetch('/setBackgroundMode?enabled='+settings.backgroundMode));promises.push(fetch('/setDirectionalLight?enabled='+settings.directionLight));promises.push(fetch('/setMotionSmoothing?enabled='+settings.motionSmoothing));Promise.all(promises).then(()=>{showSavedNotification();alert('All advanced settings saved successfully!')}).catch(error=>{console.error('Error saving settings:',error);alert('Some settings failed to save. Please try again.')})}
function loadAdvancedSettings(){fetch('/settings').then(r=>r.json()).then(data=>{document.getElementById('lightSpan').value=data.movingLightSpan||40;document.getElementById('lightSpanValue').textContent=data.movingLightSpan||40;document.getElementById('trailLength').value=data.trailLength||0;document.getElementById('trailLengthValue').textContent=data.trailLength||0;document.getElementById('centerShift').value=data.centerShift||0;document.getElementById('centerShiftValue').textContent=data.centerShift||0;document.getElementById('positionSmoothing').value=Math.round((data.positionSmoothingFactor||0.2)*100);document.getElementById('positionSmoothingValue').textContent=Math.round((data.positionSmoothingFactor||0.2)*100);document.getElementById('velocitySmoothing').value=Math.round((data.velocitySmoothingFactor||0.1)*100);document.getElementById('velocitySmoothingValue').textContent=Math.round((data.velocitySmoothingFactor||0.1)*100);document.getElementById('predictionFactor').value=Math.round((data.predictionFactor||0.5)*100);document.getElementById('predictionFactorValue').textContent=Math.round((data.predictionFactor||0.5)*100);document.getElementById('positionPGain').value=Math.round((data.positionPGain||0.1)*100);document.getElementById('positionPGainValue').textContent=Math.round((data.positionPGain||0.1)*100);document.getElementById('positionIGain').value=Math.round((data.positionIGain||0.01)*100);document.getElementById('positionIGainValue').textContent=Math.round((data.positionIGain||0.01)*100);if(document.getElementById('backgroundMode'))document.getElementById('backgroundMode').checked=data.backgroundMode||false;if(document.getElementById('directionLight'))document.getElementById('directionLight').checked=data.directionLightEnabled||false;if(document.getElementById('motionSmoothing'))document.getElementById('motionSmoothing').checked=data.motionSmoothingEnabled||false;setupAdvancedSliders()}).catch(e=>console.error('Error loading advanced settings:',e))}
function setupAdvancedSliders(){const sliders=document.querySelectorAll('#lightSpan,#positionSmoothing,#velocitySmoothing,#predictionFactor,#positionPGain,#positionIGain');sliders.forEach(slider=>{const valueDisplay=document.getElementById(slider.id+'Value');slider.oninput=function(){valueDisplay.textContent=this.value}})}
function addSelectedSlave(){const selected=document.querySelector('input[name="selectedSlave"]:checked');if(selected){addSlave(selected.value)}else{alert('Please select a slave device first')}}
function removeSelectedSlave(){const selected=document.querySelector('input[name="selectedNode"]:checked');if(selected){removeSlave(selected.value)}else{alert('Please select a node to remove')}}
function updateLEDPreview(distance,percentage){const previewContainer=document.getElementById('ledPreviewStrip');if(!previewContainer)return;const numLeds=parseInt(document.getElementById('numLeds')?.value||'300');const lightSpan=parseInt(document.getElementById('lightSpan')?.value||'40');const redValue=parseInt(document.getElementById('redValue')?.value||'255');const greenValue=parseInt(document.getElementById('greenValue')?.value||'255');const blueValue=parseInt(document.getElementById('blueValue')?.value||'255');const brightness=parseInt(document.getElementById('brightness')?.value||'255')/255;const directionLight=document.getElementById('directionLight')?.checked||false;const backgroundMode=document.getElementById('backgroundMode')?.checked||false;const lightMode=parseInt(document.getElementById('lightMode')?.value||'0');previewContainer.innerHTML='';const pixelsToShow=Math.min(100,numLeds);const pixelWidth=Math.max(2,Math.floor(previewContainer.offsetWidth/pixelsToShow)-1);const time=Date.now()/1000;for(let i=0;i<pixelsToShow;i++){const pixel=document.createElement('div');pixel.className='led-pixel';pixel.style.left=(i*(pixelWidth+1))+'px';pixel.style.width=pixelWidth+'px';const globalPos=Math.floor((i/pixelsToShow)*numLeds);const centerPos=numLeds-Math.floor((percentage/100)*numLeds);const distanceFromCenter=Math.abs(globalPos-centerPos);let finalRed=0,finalGreen=0,finalBlue=0;let intensity=0;if(lightMode===0){if(directionLight){if(distanceFromCenter<=lightSpan/2){intensity=1-((distanceFromCenter/(lightSpan/2))*0.7)}}else{if(globalPos<=centerPos&&globalPos>=centerPos-lightSpan){const fadePos=(centerPos-globalPos)/lightSpan;intensity=Math.max(0.1,1-fadePos)}}if(backgroundMode&&intensity<0.1){intensity=0.05}finalRed=Math.floor(redValue*intensity*brightness);finalGreen=Math.floor(greenValue*intensity*brightness);finalBlue=Math.floor(blueValue*intensity*brightness)}else if(lightMode===1){const hue=(globalPos*360/numLeds+time*50)%360;const rgb=hslToRgb(hue/360,1,0.5);finalRed=Math.floor(rgb[0]*brightness);finalGreen=Math.floor(rgb[1]*brightness);finalBlue=Math.floor(rgb[2]*brightness)}else if(lightMode===2){const wave=Math.sin((globalPos/20)+time*2)*0.5+0.5;finalRed=Math.floor(redValue*wave*brightness);finalGreen=Math.floor(greenValue*wave*brightness);finalBlue=Math.floor(blueValue*wave*brightness)}else if(lightMode===3){const breathe=Math.sin(time*2)*0.5+0.5;finalRed=Math.floor(redValue*breathe*brightness);finalGreen=Math.floor(greenValue*breathe*brightness);finalBlue=Math.floor(blueValue*breathe*brightness)}else if(lightMode===4){finalRed=Math.floor(redValue*brightness);finalGreen=Math.floor(greenValue*brightness);finalBlue=Math.floor(blueValue*brightness)}else if(lightMode===7){const flicker=Math.random()*0.3+0.7;const heat=Math.max(0,Math.sin(globalPos/10+time)*0.5+0.5);finalRed=Math.floor(255*heat*flicker*brightness);finalGreen=Math.floor(100*heat*flicker*brightness);finalBlue=0}pixel.style.backgroundColor=`rgb(${finalRed},${finalGreen},${finalBlue})`;previewContainer.appendChild(pixel)}const infoBar=document.getElementById('ledInfoBar');if(infoBar){const activePosition=numLeds-Math.floor((percentage/100)*numLeds);const modeName=['Standard','Rainbow','Color Wave','Breathing','Solid','Comet','Pulse','Fire','Theater','Dual Scan','Motion Particles'][lightMode]||'Unknown';infoBar.innerHTML=`<span>LEDs: <span class="led-count-display">${numLeds}</span></span><span>Mode: <span class="led-position-display">${modeName}</span></span><span>Distance: <span class="led-position-display">${distance}cm</span></span>`}}
function hslToRgb(h,s,l){let r,g,b;if(s===0){r=g=b=l}else{const hue2rgb=(p,q,t)=>{if(t<0)t+=1;if(t>1)t-=1;if(t<1/6)return p+(q-p)*6*t;if(t<1/2)return q;if(t<2/3)return p+(q-p)*(2/3-t)*6;return p};const q=l<0.5?l*(1+s):l+s-l*s;const p=2*l-q;r=hue2rgb(p,q,h+1/3);g=hue2rgb(p,q,h);b=hue2rgb(p,q,h-1/3)}return[Math.round(r*255),Math.round(g*255),Math.round(b*255)]}
function initLEDPreview(){const previewContainer=document.getElementById('ledPreviewContainer');if(!previewContainer)return;let previewHTML=`<div class="led-preview-header"><div class="led-preview-title">🎨 LED Strip Preview</div><div class="led-preview-controls"><button class="led-preview-btn active" onclick="setPreviewMode('standard')">Standard</button><button class="led-preview-btn" onclick="setPreviewMode('mesh')">Mesh</button></div></div>`;previewHTML+=`<div class="led-strip-visual" id="ledPreviewStrip"></div>`;previewHTML+=`<div class="led-info-bar" id="ledInfoBar"></div>`;previewHTML+=`<div class="mesh-segment-preview" id="meshPreview" style="display:none;"><div class="segment-label">Master Device (LEDs 0-99)</div><div class="segment-strip" id="masterSegment"></div><div class="segment-label">Slave 1 (LEDs 100-199)</div><div class="segment-strip segment-active" id="slave1Segment"></div><div class="segment-label">Slave 2 (LEDs 200-299)</div><div class="segment-strip" id="slave2Segment"></div></div>`;previewContainer.innerHTML=previewHTML;updateLEDPreview(parseInt(document.getElementById('liveDistance')?.textContent||'150'),50)}
function setPreviewMode(mode){const standardBtn=document.querySelector('[onclick="setPreviewMode(\'standard\')"]');const meshBtn=document.querySelector('[onclick="setPreviewMode(\'mesh\')"]');const standardView=document.getElementById('ledPreviewStrip');const meshView=document.getElementById('meshPreview');if(mode==='standard'){standardBtn.classList.add('active');meshBtn.classList.remove('active');standardView.style.display='block';meshView.style.display='none'}else{standardBtn.classList.remove('active');meshBtn.classList.add('active');standardView.style.display='none';meshView.style.display='block';updateMeshPreview()}}
function updateMeshPreview(){fetch('/sensordata').then(r=>r.json()).then(data=>{const segments=[{id:'masterSegment',label:'Master Device'},{id:'slave1Segment',label:'Slave 1'},{id:'slave2Segment',label:'Slave 2'}];segments.forEach((segmentInfo,index)=>{const segment=document.getElementById(segmentInfo.id);if(!segment)return;segment.innerHTML='';const pixelsToShow=25;const pixelWidth=Math.floor(segment.offsetWidth/pixelsToShow)-1;const sensorData=data.sensors?data.sensors.find(s=>s.id===index):null;const isActive=sensorData&&sensorData.active;const distance=sensorData?sensorData.distance:0;const segmentStart=index*100;const segmentLength=100;for(let i=0;i<pixelsToShow;i++){const pixel=document.createElement('div');pixel.className='led-pixel';pixel.style.left=(i*(pixelWidth+1))+'px';pixel.style.width=pixelWidth+'px';const globalPos=segmentStart+Math.floor((i/pixelsToShow)*segmentLength);let intensity=0;let red=20,green=20,blue=20;if(isActive&&distance>0){const centerPos=segmentStart+segmentLength-Math.floor((distance/300)*segmentLength);const distanceFromCenter=Math.abs(globalPos-centerPos);const lightSpan=40;if(distanceFromCenter<=lightSpan/2){intensity=1-((distanceFromCenter/(lightSpan/2))*0.7);red=Math.floor(255*intensity);green=Math.floor(255*intensity);blue=Math.floor(255*intensity)}}else if(index===0&&Math.random()>0.9){intensity=0.2;red=Math.floor(100*intensity);green=Math.floor(150*intensity);blue=Math.floor(255*intensity)}pixel.style.backgroundColor=`rgb(${red},${green},${blue})`;segment.appendChild(pixel)}const parentDiv=segment.parentElement;const label=parentDiv.querySelector('.segment-label');if(label){if(isActive){label.innerHTML=`${segmentInfo.label} (LEDs ${segmentStart}-${segmentStart+segmentLength-1}) - ACTIVE: ${distance}cm`;parentDiv.classList.add('segment-active')}else{label.innerHTML=`${segmentInfo.label} (LEDs ${segmentStart}-${segmentStart+segmentLength-1}) - Idle`;parentDiv.classList.remove('segment-active')}}})}).catch(e=>{console.error('Error fetching sensor data for mesh preview:',e)})}}
function triggerLEDPreviewUpdate(){setTimeout(()=>{const currentDistance=parseInt(document.getElementById('liveDistance')?.textContent?.replace('cm','')||'150');const minDist=parseInt(document.getElementById('minDist')?.value||'30');const maxDist=parseInt(document.getElementById('maxDist')?.value||'300');const percentage=Math.min(100,Math.max(0,((currentDistance-minDist)/(maxDist-minDist))*100));updateLEDPreview(currentDistance,percentage);const activeMode=document.querySelector('.led-preview-btn.active')?.textContent;if(activeMode==='Mesh'){updateMeshPreview()}},100)}
function setupPreviewUpdates(){const controls=['numLeds','brightness','redValue','greenValue','blueValue','lightSpan','lightMode','directionLight','backgroundMode'];controls.forEach(controlId=>{const element=document.getElementById(controlId);if(element){if(element.type==='checkbox'){element.addEventListener('change',triggerLEDPreviewUpdate)}else{element.addEventListener('input',triggerLEDPreviewUpdate)}}})}
document.addEventListener('DOMContentLoaded',function(){updateDistance();if(document.getElementById('lightSpan')){loadAdvancedSettings()}initLEDPreview();setupPreviewUpdates();document.getElementById('loadingOverlay').style.display='none'})
)literal";

// Base HTML template with placeholders
const char html_template_full[] PROGMEM = R"literal(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>AmbiSense | TechPosts Media</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>%CSS%</style>
</head>
<body>
<div class="loading-overlay" id="loadingOverlay">
<div class="spinner"></div>
</div>
<div class="main-content">
<div class="container">
<div class="header">
<h1>AmbiSense</h1>
<p>Radar-controlled LED strip configuration</p>
</div>
<div class="content">
<div class="tab-container">
<button class="tab %BASIC_TAB_ACTIVE%" onclick="window.location.href='/'">Basic</button>
<button class="tab %ADVANCED_TAB_ACTIVE%" onclick="window.location.href='/advanced'">Advanced</button>
<button class="tab %EFFECTS_TAB_ACTIVE%" onclick="window.location.href='/effects'">Effects</button>
<button class="tab %MESH_TAB_ACTIVE%" onclick="window.location.href='/mesh'">Multi-Sensor</button>
<button class="tab %CALIBRATION_TAB_ACTIVE%" onclick="window.location.href='/calibration'">Calibration</button>
<button class="tab %NETWORK_TAB_ACTIVE%" onclick="window.location.href='/network'">Network</button>
<button class="tab %DIAGNOSTICS_TAB_ACTIVE%" onclick="window.location.href='/diagnostics'">Diagnostics</button>
</div>
%TAB_CONTENT%
<div class="led-preview-container" id="ledPreviewContainer">
<!-- LED Preview will be dynamically generated -->
</div>
<div class="distance-display">
<div class="distance-label">Current Distance</div>
<div class="distance-value" id="liveDistance">--<span class="distance-unit">cm</span></div>
<div class="led-visualizer">
<div class="led-active" id="ledVisualizer" style="width: 0%;"></div>
</div>
</div>
</div>
</div>
</div>
<div class="footer">
<p class="branding">AmbiSense by Ravi Singh (TechPosts Media)</p>
<p>Radar-Controlled LED System</p>
<p>Copyright &copy; 2025 TechPosts Media. All rights reserved.</p>
</div>
<div class="settings-saved" id="settingsSaved">
Settings saved successfully!
</div>
<script>%JS%</script>
%TAB_SCRIPTS%
</body>
</html>)literal";

// Complete Basic tab with all original functionality
const char basic_tab_full[] PROGMEM = R"literal(
<div class="form-group">
<label>Number of LEDs</label>
<div class="led-count-container">
<input type="number" min="1" max="2000" class="led-count-input" id="numLeds" value="300">
<div class="slider-value"><span id="ledDensity">5</span>m</div>
</div>
</div>
<div class="form-group">
<label>Minimum Distance (cm)</label>
<div class="slider-container">
<input type="range" min="0" max="500" value="30" class="slider" id="minDist">
<div class="slider-value" id="minDistValue">30</div>
</div>
</div>
<div class="form-group">
<label>Maximum Distance (cm)</label>
<div class="slider-container">
<input type="range" min="50" max="500" value="300" class="slider" id="maxDist">
<div class="slider-value" id="maxDistValue">300</div>
</div>
</div>
<div class="form-group">
<label>LED Color</label>
<div style="display: flex; gap: 15px; margin-bottom: 15px;">
<div class="color-preview" id="colorPreview"></div>
</div>
<div class="rgb-sliders">
<div class="slider-container">
<span class="rgb-label r">R</span>
<input type="range" min="0" max="255" value="255" class="slider" id="redValue">
<div class="slider-value" id="redValueDisplay">255</div>
</div>
<div class="slider-container">
<span class="rgb-label g">G</span>
<input type="range" min="0" max="255" value="255" class="slider" id="greenValue">
<div class="slider-value" id="greenValueDisplay">255</div>
</div>
<div class="slider-container">
<span class="rgb-label b">B</span>
<input type="range" min="0" max="255" value="255" class="slider" id="blueValue">
<div class="slider-value" id="blueValueDisplay">255</div>
</div>
</div>
</div>
<div class="form-group">
<label>Brightness (0-255)</label>
<div class="slider-container">
<input type="range" min="0" max="255" value="255" class="slider" id="brightness">
<div class="slider-value" id="brightnessValue">255</div>
</div>
</div>
<div style="text-align: center; margin-top: 20px;">
<a href="/resetdistance" style="display: inline-block; padding: 10px; background: #ff4d4d; color: white; text-decoration: none; border-radius: 5px;">
Reset Min/Max Distance to Defaults
</a>
</div>
<button class="button" id="saveButton">Save Settings</button>
)literal";

// Basic tab JavaScript
const char basic_tab_js[] PROGMEM = R"literal(
<script>
window.onload=function(){loadCurrentSettings();const sliders=document.querySelectorAll('.slider');sliders.forEach(slider=>{const valueDisplay=document.getElementById(slider.id+(slider.id.includes('Value')?'Display':'Value'));slider.oninput=function(){valueDisplay.textContent=this.value;if(['redValue','greenValue','blueValue'].includes(this.id)){updateColorPreview()}};slider.addEventListener('change',function(){autoSaveChanges(this.id,this.value)})});const numLedsInput=document.getElementById('numLeds');numLedsInput.oninput=function(){const lengthInMeters=(parseInt(this.value)/60).toFixed(1);document.getElementById('ledDensity').textContent=lengthInMeters;updateColorPreview()};numLedsInput.addEventListener('change',function(){autoSaveChanges('numLeds',this.value)});document.getElementById('saveButton').addEventListener('click',function(){const numLeds=document.getElementById('numLeds').value;const minDist=document.getElementById('minDist').value;const maxDist=document.getElementById('maxDist').value;const brightness=document.getElementById('brightness').value;const redValue=document.getElementById('redValue').value;const greenValue=document.getElementById('greenValue').value;const blueValue=document.getElementById('blueValue').value;fetch('/set?numLeds='+numLeds+'&minDist='+minDist+'&maxDist='+maxDist+'&brightness='+brightness+'&redValue='+redValue+'&greenValue='+greenValue+'&blueValue='+blueValue).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error("Error saving settings:",error)})})};
function loadCurrentSettings(){fetch('/settings').then(response=>response.json()).then(settings=>{document.getElementById('numLeds').value=settings.numLeds;document.getElementById('ledDensity').textContent=(settings.numLeds/60).toFixed(1);document.getElementById('minDist').value=settings.minDistance;document.getElementById('minDistValue').textContent=settings.minDistance;document.getElementById('maxDist').value=settings.maxDistance;document.getElementById('maxDistValue').textContent=settings.maxDistance;document.getElementById('brightness').value=settings.brightness;document.getElementById('brightnessValue').textContent=settings.brightness;document.getElementById('redValue').value=settings.redValue;document.getElementById('redValueDisplay').textContent=settings.redValue;document.getElementById('greenValue').value=settings.greenValue;document.getElementById('greenValueDisplay').textContent=settings.greenValue;document.getElementById('blueValue').value=settings.blueValue;document.getElementById('blueValueDisplay').textContent=settings.blueValue;updateColorPreview();document.getElementById('loadingOverlay').style.display='none'}).catch(error=>{console.error('Error loading settings:',error);document.getElementById('loadingOverlay').style.display='none'})}
</script>
)literal";

// Advanced tab - Effect & Motion Tuning
const char advanced_tab_full[] PROGMEM = R"literal(
<style>
.section-header{font-size:16px;font-weight:600;color:var(--primary);margin:20px 0 15px 0;padding-bottom:8px;border-bottom:2px solid var(--primary);display:flex;align-items:center}
.section-header::before{margin-right:8px}
.effect-section{background:#2a2a2a;padding:20px;margin:15px 0;border-radius:10px;border-left:4px solid var(--primary)}
</style>

<div class="section-header" style="--icon:'🔦'">🔦 Effect Shape</div>
<div class="form-group">
<label>Light Span</label>
<div class="slider-container">
<input type="range" min="1" max="100" value="40" class="slider" id="lightSpan" onchange="autoSaveChanges('lightSpan', this.value)">
<div class="slider-value" id="lightSpanValue">40</div>
</div>
<small class="input-description">Controls the width of the moving light effect</small>
</div>
<div class="form-group">
<label>Trail Length</label>
<div class="slider-container">
<input type="range" min="0" max="20" value="0" class="slider" id="trailLength" onchange="fetch('/setTrailLength?value='+this.value).then(r=>r.json()).then(d=>{if(d.status==='success')showSavedNotification()})">
<div class="slider-value" id="trailLengthValue">0</div>
</div>
<small class="input-description">Length of the trailing light effect</small>
</div>
<div class="form-group">
<label>Center Shift</label>
<div class="slider-container">
<input type="range" min="-50" max="50" value="0" class="slider" id="centerShift" onchange="fetch('/setCenterShift?value='+this.value).then(r=>r.json()).then(d=>{if(d.status==='success')showSavedNotification()})">
<div class="slider-value" id="centerShiftValue">0</div>
</div>
<small class="input-description">Shift the center position of the effect</small>
</div>

<div class="section-header">🔁 Smoothing & Prediction</div>
<div class="form-group">
<label>Position Smoothing</label>
<div class="slider-container">
<input type="range" min="0" max="100" value="20" class="slider" id="positionSmoothing" onchange="updateMotionParam('positionSmoothingFactor', this.value/100)">
<div class="slider-value" id="positionSmoothingValue">20</div>
</div>
<small class="input-description">Smooths position readings to reduce jitter</small>
</div>
<div class="form-group">
<label>Velocity Smoothing</label>
<div class="slider-container">
<input type="range" min="0" max="100" value="10" class="slider" id="velocitySmoothing" onchange="updateMotionParam('velocitySmoothingFactor', this.value/100)">
<div class="slider-value" id="velocitySmoothingValue">10</div>
</div>
<small class="input-description">Smooths velocity calculations</small>
</div>
<div class="form-group">
<label>Prediction Factor</label>
<div class="slider-container">
<input type="range" min="0" max="100" value="50" class="slider" id="predictionFactor" onchange="updateMotionParam('predictionFactor', this.value/100)">
<div class="slider-value" id="predictionFactorValue">50</div>
</div>
<small class="input-description">Predicts future position based on velocity</small>
</div>

<div class="section-header">🎯 PID Gains</div>
<div class="form-group">
<label>Position P Gain</label>
<div class="slider-container">
<input type="range" min="0" max="100" value="10" class="slider" id="positionPGain" onchange="updateMotionParam('positionPGain', this.value/100)">
<div class="slider-value" id="positionPGainValue">10</div>
</div>
<small class="input-description">Proportional gain for position control</small>
</div>
<div class="form-group">
<label>Position I Gain</label>
<div class="slider-container">
<input type="range" min="0" max="10" value="1" class="slider" id="positionIGain" onchange="updateMotionParam('positionIGain', this.value/100)">
<div class="slider-value" id="positionIGainValue">1</div>
</div>
<small class="input-description">Integral gain for position control</small>
</div>

<div class="section-header">🧪 Toggles</div>
<div class="toggle-container">
<span class="toggle-label">Background Mode</span>
<label class="toggle-switch">
<input type="checkbox" id="backgroundMode" onchange="setBackgroundMode(this.checked)">
<span class="toggle-slider"></span>
</label>
</div>
<div class="toggle-container">
<span class="toggle-label">Directional Light</span>
<label class="toggle-switch">
<input type="checkbox" id="directionLight" onchange="setDirectionalLight(this.checked)">
<span class="toggle-slider"></span>
</label>
</div>
<div class="toggle-container">
<span class="toggle-label">Motion Smoothing</span>
<label class="toggle-switch">
<input type="checkbox" id="motionSmoothing" onchange="setMotionSmoothing(this.checked)">
<span class="toggle-slider"></span>
</label>
</div>

<div class="section-header">💾 Save Settings</div>
<button class="button" onclick="saveAdvancedSettings()">Save All Advanced Settings</button>
)literal";

// Effects tab with complete functionality
const char effects_tab_full[] PROGMEM = R"literal(
<div class="form-group">
<label>Light Mode</label>
<select id="lightMode" onchange="setLightMode(this.value)" style="width:100%;padding:12px;border-radius:8px;background:#333;color:#fff;border:none;font-size:14px;">
<option value="0">Standard</option>
<option value="1">Rainbow</option>
<option value="2">Color Wave</option>
<option value="3">Breathing</option>
<option value="4">Solid</option>
<option value="5">Comet</option>
<option value="6">Pulse</option>
<option value="7">Fire</option>
<option value="8">Theater Chase</option>
<option value="9">Dual Scan</option>
<option value="10">Motion Particles</option>
</select>
</div>
<div class="form-group">
<label>Effect Speed</label>
<div class="slider-container">
<input type="range" min="1" max="100" value="50" class="slider" id="effectSpeed" onchange="setEffectSpeed(this.value)">
<div class="slider-value" id="effectSpeedValue">50</div>
</div>
</div>
<div class="form-group">
<label>Effect Intensity</label>
<div class="slider-container">
<input type="range" min="1" max="100" value="50" class="slider" id="effectIntensity" onchange="setEffectIntensity(this.value)">
<div class="slider-value" id="effectIntensityValue">50</div>
</div>
</div>
)literal";

// Multi-Sensor tab - Mesh Setup & Monitoring
const char mesh_tab_full[] PROGMEM = R"literal(
<style>
.section-header{font-size:16px;font-weight:600;color:var(--primary);margin:20px 0 15px 0;padding-bottom:8px;border-bottom:2px solid var(--primary);display:flex;align-items:center}
.mesh-section{background:#2a2a2a;padding:20px;margin:15px 0;border-radius:10px;border-left:4px solid var(--primary)}
.sensor-table{width:100%;border-collapse:collapse;margin:15px 0;background:#333;border-radius:8px;overflow:hidden}
.sensor-table th{background:var(--primary);color:white;padding:12px 8px;text-align:left;font-size:12px;font-weight:600}
.sensor-table td{padding:10px 8px;border-bottom:1px solid #444;font-size:11px}
.sensor-table tr:last-child td{border-bottom:none}
.sensor-table tr:hover{background:#404040}
.sensor-table tr.active-sensor{background:#2a4a4a;border-left:4px solid #4cc9f0}
.signal-strong{color:#4cc9f0}
.signal-medium{color:#ffd60a}
.signal-weak{color:#ff6b6b}
.status-active{color:#4cc9f0}
.status-inactive{color:#666}
.scan-results{max-height:200px;overflow-y:auto;background:#333;border-radius:8px;padding:10px;margin:10px 0}
.device-item{background:#404040;margin:5px 0;padding:10px;border-radius:5px;display:flex;justify-content:space-between;align-items:center}
.device-name{font-weight:600;color:var(--text)}
.device-mac{font-size:10px;color:#999;font-family:monospace}
.device-signal{font-size:10px;padding:2px 6px;border-radius:3px}
.btn-small{padding:6px 12px;font-size:11px;border-radius:4px;border:none;cursor:pointer;margin:0 2px}
.btn-add{background:#4cc9f0;color:white}
.btn-remove{background:#ff6b6b;color:white}
.segment-config{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:10px 0}
.realtime-monitor{background:#1a1a1a;padding:15px;border-radius:8px;margin:15px 0}
.distance-card{background:#333;padding:10px;border-radius:6px;margin:5px 0;display:flex;justify-content:space-between;align-items:center}
.distance-value{font-size:18px;font-weight:bold;color:var(--success)}
.distance-label{font-size:11px;color:#999}
.priority-indicator{width:8px;height:8px;border-radius:50%;margin-right:8px}
.priority-active{background:#4cc9f0;box-shadow:0 0 6px #4cc9f0}
.priority-inactive{background:#666}
</style>

<div style="text-align:center;margin-bottom:30px;">
<button class="button" onclick="window.location.href='/wizard'" style="background:linear-gradient(135deg,var(--success) 0%,var(--primary) 100%);padding:15px 30px;font-size:16px;border-radius:10px;box-shadow:0 5px 15px rgba(76,201,240,0.4);">
🧙‍♂️ Setup Wizard for L/U Stairs
</button>
<div style="font-size:12px;color:var(--text-secondary);margin-top:8px;">Automatic configuration for complex stair layouts</div>
</div>

<div class="section-header">🧭 Manual Device Configuration</div>
<div class="form-group">
<label>Device Role</label>
<select id="deviceRole" onchange="updateDeviceRole(this.value)" style="width:100%;padding:12px;border-radius:8px;background:#333;color:#fff;border:none;font-size:14px;">
<option value="1">Master</option>
<option value="2">Slave</option>
</select>
<small class="input-description">Master aggregates sensor data from multiple Slaves and controls LED effects</small>
</div>

<div class="form-group">
<label>Device MAC Address</label>
<div style="display:flex;gap:10px;align-items:center;">
<div style="flex:1;padding:10px;background:#333;border-radius:6px;font-family:monospace;font-size:14px;color:#4cc9f0;text-align:center;border:2px solid #4cc9f0;">
<span id="deviceMac">Loading...</span>
</div>
<button onclick="copyMacAddress()" style="padding:10px;background:#4361ee;color:white;border:none;border-radius:6px;cursor:pointer;font-size:12px;min-width:60px;">Copy</button>
</div>
<small class="input-description" id="macDescription">This device's MAC address for ESP-NOW pairing</small>
</div>

<div class="form-group" id="masterMacSection" style="display:none;">
<label>Master MAC Address</label>
<div style="display:flex;gap:10px;">
<input type="text" id="masterMacInput" placeholder="XX:XX:XX:XX:XX:XX" maxlength="17" style="flex:1;padding:10px;border-radius:6px;background:#333;color:#fff;border:none;font-family:monospace;text-transform:uppercase;">
<button onclick="setMasterMac()" style="padding:10px 20px;background:#4cc9f0;color:#000;border:none;border-radius:6px;font-weight:600;cursor:pointer;">Connect</button>
</div>
<div id="connectionStatus" style="margin-top:10px;padding:8px;border-radius:6px;display:none;">
<div id="connectionStatusText" style="font-size:14px;"></div>
</div>
<small class="input-description">Enter the Master device's MAC address to establish ESP-NOW connection</small>
</div>

<div class="section-header">🧱 Segment Settings</div>
<div class="form-group">
<label>Segment Mode</label>
<select id="ledSegmentMode" onchange="updateLEDMode(this.value)" style="width:100%;padding:12px;border-radius:8px;background:#333;color:#fff;border:none;font-size:14px;">
<option value="0">Continuous</option>
<option value="1">Distributed</option>
</select>
<small class="input-description">Continuous: single strip, Distributed: multiple segments</small>
</div>
<div id="segmentConfig" class="segment-config" style="display:none;">
<div class="form-group">
<label>Segment Start</label>
<input type="number" id="segmentStart" min="0" placeholder="Start LED" style="width:100%;padding:10px;border-radius:6px;background:#333;color:#fff;border:none;">
</div>
<div class="form-group">
<label>Segment Length</label>
<input type="number" id="segmentLength" min="1" placeholder="LED Count" style="width:100%;padding:10px;border-radius:6px;background:#333;color:#fff;border:none;">
</div>
</div>

<div class="section-header">📡 ESP-NOW Mesh</div>
<div style="display:flex;gap:10px;margin-bottom:15px;">
<button class="button" onclick="scanForAmbiSenseDevices()" style="flex:1;padding:10px;">Scan</button>
<button class="button" onclick="addSelectedSlave()" style="flex:1;padding:10px;background:#4cc9f0;">Add Slave</button>
<button class="button" onclick="removeSelectedSlave()" style="flex:1;padding:10px;background:#ff6b6b;">Remove</button>
</div>
<div id="scanResults" class="scan-results" style="display:none;">
<div id="scanList"></div>
</div>
<div class="form-group">
<label>Known Nodes</label>
<table class="sensor-table" id="slavesTable">
<thead>
<tr><th>Node</th><th>MAC Address</th><th>Signal</th><th>Status</th></tr>
</thead>
<tbody id="slavesTableBody">
<tr><td colspan="4" style="text-align:center;color:#666">No nodes connected</td></tr>
</tbody>
</table>
</div>

<div class="section-header">📊 Real-Time Distance Monitor</div>
<div class="realtime-monitor">
<table class="sensor-table" id="distanceTable">
<thead>
<tr><th>Sensor ID</th><th>Distance</th><th>Signal</th><th>Last Update</th></tr>
</thead>
<tbody id="distanceTableBody">
<tr><td>A4:12:6D:23:...</td><td>75 cm</td><td><span class="signal-strong">Strong</span></td><td>0.2s ago</td></tr>
<tr class="active-sensor"><td>B8:D6:1A:91:...</td><td><strong>126 cm</strong></td><td><span class="signal-medium">Medium</span></td><td>0.4s ago</td></tr>
<tr><td>C2:4F:8B:A5:...</td><td>--</td><td><span class="signal-weak">Weak</span></td><td>2.1s ago</td></tr>
</tbody>
</table>
<div style="margin-top:15px;padding:10px;background:#1a4a4a;border-radius:6px;border-left:4px solid #4cc9f0;">
<div style="font-size:12px;color:#999;">🟢 Active Sensor (driving effect)</div>
<div style="font-size:16px;font-weight:bold;color:#4cc9f0;">B8:D6:1A:91:... - 126 cm</div>
</div>
</div>

<div style="text-align:center;margin-top:20px;">
<button class="button" onclick="window.location.href='/diagnostics'">📊 View Full Diagnostics</button>
</div>
)literal";

// Network tab
const char network_tab_full[] PROGMEM = R"literal(
<div style="background:#333;padding:15px;border-radius:8px;margin-bottom:20px;">
<p><strong>Status:</strong> <span id="wifi_status">Loading...</span></p>
<p><strong>IP:</strong> <span id="wifi_ip">Loading...</span></p>
<p><strong>Device Role:</strong> <span id="device_role">Loading...</span></p>
</div>

<div id="slaveNetworkMessage" style="display:none;background:#2a2a2a;padding:20px;border-radius:8px;margin-bottom:20px;border-left:4px solid #ff9500;">
<h3 style="color:#ff9500;margin:0 0 15px 0;">🔒 Network Configuration Restricted</h3>
<p style="margin:0 0 10px 0;color:#b0b0b0;">This device is configured as a <strong>Slave</strong> in the ESP-NOW mesh network.</p>
<p style="margin:0 0 15px 0;color:#b0b0b0;">Network settings are managed by the <strong>Master</strong> device to ensure proper mesh communication and synchronized operation.</p>
<div style="background:#333;padding:10px;border-radius:6px;margin:10px 0;">
<p style="margin:0;font-size:14px;color:#999;"><strong>To modify network settings:</strong></p>
<p style="margin:5px 0 0 0;font-size:14px;color:#999;">1. Change device role to Master in the Multi-Sensor tab, or</p>
<p style="margin:0;font-size:14px;color:#999;">2. Configure network settings on the Master device</p>
</div>
</div>

<div id="networkConfigForm">
<form method="post" action="/network">
<div class="form-group">
<label>WiFi Network</label>
<button type="button" onclick="scanWifiNetworks()" class="button" style="width:100%;padding:10px 15px;margin:0 0 10px 0;font-size:14px;background:#4895ef;">📡 Scan Networks</button>
<select id="networkSelect" onchange="selectNetwork()" style="width:100%;padding:12px;border-radius:8px;background:#333;color:#fff;border:none;font-size:14px;margin-bottom:10px;">
<option value="">🔍 Click "Scan Networks" to find available WiFi</option>
</select>
<input type="text" id="ssidInput" name="ssid" placeholder="Or enter SSID manually" required style="width:100%;padding:12px;border-radius:8px;background:#333;color:#fff;border:none;font-size:14px;">
</div>
<div class="form-group">
<label>Password</label>
<input type="password" name="password" placeholder="Enter Password" style="width:100%;padding:12px;border-radius:8px;background:#333;color:#fff;border:none;font-size:14px;">
</div>
<div class="form-group">
<label>Device Name</label>
<input type="text" name="deviceName" placeholder="Enter Device Name" required style="width:100%;padding:12px;border-radius:8px;background:#333;color:#fff;border:none;font-size:14px;">
</div>
<button type="submit" class="button">Save Network Settings</button>
</form>
<button onclick="location.href='/resetwifi'" class="button" style="background:#ef4444;margin-top:20px;">Reset WiFi Settings</button>
</div>
)literal";

const char network_tab_js[] PROGMEM = R"literal(
<script>
// Load network status and device role
Promise.all([
  fetch('/settings').catch(() => ({})),
  fetch('/getDeviceInfo').catch(() => ({}))
]).then(([settingsResponse, deviceResponse]) => {
  // Handle WiFi status
  if (settingsResponse.ok) {
    document.getElementById('wifi_status').textContent = 'Connected';
    document.getElementById('wifi_ip').textContent = location.hostname;
  } else {
    document.getElementById('wifi_status').textContent = 'AP Mode';
    document.getElementById('wifi_ip').textContent = location.hostname;
  }

  // Handle device role and conditional network form display
  if (deviceResponse.ok) {
    deviceResponse.json().then(deviceData => {
      const role = deviceData.role;
      const roleText = role == 1 ? 'Master' : role == 2 ? 'Slave' : 'Unknown';
      document.getElementById('device_role').textContent = roleText;

      // Show/hide network configuration based on device role
      if (role == 2) { // Slave device
        document.getElementById('networkConfigForm').style.display = 'none';
        document.getElementById('slaveNetworkMessage').style.display = 'block';
      } else { // Master device or unknown
        document.getElementById('networkConfigForm').style.display = 'block';
        document.getElementById('slaveNetworkMessage').style.display = 'none';
      }
    }).catch(() => {
      document.getElementById('device_role').textContent = 'Unknown';
      // Default to showing network config if we can't determine role
      document.getElementById('networkConfigForm').style.display = 'block';
      document.getElementById('slaveNetworkMessage').style.display = 'none';
    });
  } else {
    document.getElementById('device_role').textContent = 'Unknown';
    // Default to showing network config if we can't determine role
    document.getElementById('networkConfigForm').style.display = 'block';
    document.getElementById('slaveNetworkMessage').style.display = 'none';
  }
}).catch(() => {
  // Fallback if both requests fail
  document.getElementById('wifi_status').textContent = 'Error';
  document.getElementById('wifi_ip').textContent = 'Unknown';
  document.getElementById('device_role').textContent = 'Unknown';
  document.getElementById('networkConfigForm').style.display = 'block';
  document.getElementById('slaveNetworkMessage').style.display = 'none';
});

function scanWifiNetworks() {
const scanBtn = document.querySelector('button[onclick="scanWifiNetworks()"]');
const networkSelect = document.getElementById('networkSelect');
scanBtn.textContent = '🔄 Scanning...';
scanBtn.disabled = true;
networkSelect.innerHTML = '<option value="">Scanning for networks...</option>';

fetch('/scannetworks')
.then(r => r.json())
.then(networks => {
networkSelect.innerHTML = '<option value="">🔍 Select a network from the list</option>';
networks.forEach(network => {
const option = document.createElement('option');
option.value = network.ssid;
const signal = network.rssi > -50 ? '📶' : network.rssi > -70 ? '📡' : '📻';
const security = network.secure ? '🔒' : '🔓';
option.textContent = `${signal} ${network.ssid} (${network.rssi}dBm) ${security}`;
networkSelect.appendChild(option);
});
scanBtn.textContent = '📡 Scan Networks';
scanBtn.disabled = false;
})
.catch(err => {
networkSelect.innerHTML = '<option value="">❌ Scan failed - click button to retry</option>';
scanBtn.textContent = '📡 Scan Networks';
scanBtn.disabled = false;
console.error('WiFi scan failed:', err);
});
}

function selectNetwork() {
const networkSelect = document.getElementById('networkSelect');
const ssidInput = document.getElementById('ssidInput');
if (networkSelect.value) {
ssidInput.value = networkSelect.value;
}
}
</script>
)literal";

// Multi-Sensor tab JavaScript functionality
const char mesh_tab_js[] PROGMEM = R"literal(
<script>
let meshUpdateInterval;
let isScanning = false;

// Initialize mesh tab functionality
function initMeshTab() {
  loadDeviceInfo();
  loadSlaveDevices();

  // Start real-time monitoring if master
  meshUpdateInterval = setInterval(updateSensorData, 1000);

  // Load segment configuration
  loadSegmentConfig();
}

// Update device role
function updateDeviceRole(role) {
  // Show/hide Master MAC input section based on role
  const masterMacSection = document.getElementById('masterMacSection');
  const macDescription = document.getElementById('macDescription');

  if (role == 2) { // Slave role
    masterMacSection.style.display = 'block';
    macDescription.textContent = 'This device\'s MAC address - share with Master device for pairing';
  } else { // Master role
    masterMacSection.style.display = 'none';
    macDescription.textContent = 'This device\'s MAC address for ESP-NOW pairing';
  }

  fetch('/setDeviceRole?role=' + role)
    .then(r => r.json())
    .then(data => {
      if (data.status === 'success') {
        showSavedNotification();
        loadDeviceInfo();
        // Show success message
        alert('Device role updated successfully!\n\nRole: ' + (role == 1 ? 'Master' : 'Slave') + '\n\nPage will refresh in 2 seconds to apply changes.');
        // Refresh page after role change
        setTimeout(() => location.reload(), 2000);
      } else {
        alert('Failed to update device role: ' + (data.message || 'Unknown error'));
        // Revert dropdown to previous value
        loadDeviceInfo();
      }
    })
    .catch(error => {
      console.error('Error updating device role:', error);
      alert('Network error updating device role. Please try again.');
      // Revert dropdown to previous value
      loadDeviceInfo();
    });
}

// Set Master MAC Address (missing function implementation)
function setMasterMac() {
  const macInput = document.getElementById('masterMacInput');
  const mac = macInput.value.trim();

  // Validate MAC address format
  const macPattern = /^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/;
  if (!mac || !macPattern.test(mac)) {
    alert('Please enter a valid MAC address in format XX:XX:XX:XX:XX:XX');
    return;
  }

  // Convert to uppercase and consistent format
  const formattedMac = mac.toUpperCase().replace(/[-]/g, ':');

  fetch('/setMasterMac?mac=' + encodeURIComponent(formattedMac))
    .then(r => r.json())
    .then(data => {
      if (data.status === 'success') {
        showSavedNotification();

        // Update connection status immediately
        const connectionStatus = document.getElementById('connectionStatus');
        const connectionStatusText = document.getElementById('connectionStatusText');
        connectionStatus.style.display = 'block';
        connectionStatus.style.background = '#1a4a4a';
        connectionStatus.style.borderLeft = '4px solid #4cc9f0';
        connectionStatusText.innerHTML = '🟢 Connected to Master: ' + formattedMac;
        connectionStatusText.style.color = '#4cc9f0';

        // Update input styling
        macInput.value = formattedMac;
        macInput.style.borderColor = '#4cc9f0';

        alert('Successfully connected to Master device!\n\nMAC: ' + formattedMac + '\n\nESP-NOW pairing established.');
        loadDeviceInfo();
      } else {
        // Show error status
        const connectionStatus = document.getElementById('connectionStatus');
        const connectionStatusText = document.getElementById('connectionStatusText');
        connectionStatus.style.display = 'block';
        connectionStatus.style.background = '#4a1a1a';
        connectionStatus.style.borderLeft = '4px solid #ff6b6b';
        connectionStatusText.innerHTML = '🔴 Connection failed: ' + (data.message || 'Unknown error');
        connectionStatusText.style.color = '#ff6b6b';

        alert('Failed to connect to Master device: ' + (data.message || 'Unknown error'));
      }
    })
    .catch(error => {
      console.error('Error setting master MAC:', error);

      // Show network error status
      const connectionStatus = document.getElementById('connectionStatus');
      const connectionStatusText = document.getElementById('connectionStatusText');
      connectionStatus.style.display = 'block';
      connectionStatus.style.background = '#4a1a1a';
      connectionStatus.style.borderLeft = '4px solid #ff6b6b';
      connectionStatusText.innerHTML = '🔴 Network error - Please try again';
      connectionStatusText.style.color = '#ff6b6b';

      alert('Failed to connect to Master device. Please check the MAC address and try again.');
    });
}

// Copy MAC address to clipboard
function copyMacAddress() {
  const macElement = document.getElementById('deviceMac');
  const mac = macElement.textContent;

  if (mac && mac !== 'Loading...') {
    navigator.clipboard.writeText(mac).then(() => {
      showSavedNotification();
      // Temporarily change button text to show it was copied
      const button = event.target;
      const originalText = button.textContent;
      button.textContent = 'Copied!';
      button.style.background = '#4cc9f0';
      setTimeout(() => {
        button.textContent = originalText;
        button.style.background = '#4361ee';
      }, 1500);
    }).catch(error => {
      console.error('Failed to copy MAC address:', error);
      // Fallback for older browsers
      const textArea = document.createElement('textarea');
      textArea.value = mac;
      document.body.appendChild(textArea);
      textArea.select();
      document.execCommand('copy');
      document.body.removeChild(textArea);
      alert('MAC address copied to clipboard: ' + mac);
    });
  } else {
    alert('MAC address not available yet');
  }
}

// Update sensor priority mode
function updateSensorPriority(mode) {
  fetch('/setSensorPriorityMode?mode=' + mode)
    .then(r => r.json())
    .then(data => {
      if (data.status === 'success') {
        showSavedNotification();
      }
    })
    .catch(error => console.error('Error updating sensor priority:', error));
}

// Update LED distribution mode
function updateLEDMode(mode) {
  const segmentConfig = document.getElementById('segmentConfig');
  if (mode == 1) {
    segmentConfig.style.display = 'grid';
  } else {
    segmentConfig.style.display = 'none';
  }

  fetch('/setLEDSegmentMode?mode=' + mode)
    .then(r => r.json())
    .then(data => {
      if (data.status === 'success') {
        showSavedNotification();
      }
    })
    .catch(error => console.error('Error updating LED mode:', error));
}

// Load device information
function loadDeviceInfo() {
  fetch('/getDeviceInfo')
    .then(r => r.json())
    .then(data => {
      // Set device role dropdown
      document.getElementById('deviceRole').value = data.role;

      // Set sensor priority mode if element exists
      const sensorPriorityElement = document.getElementById('sensorPriorityMode');
      if (sensorPriorityElement) {
        sensorPriorityElement.value = data.sensorPriorityMode || 0;
      }

      // Display device MAC address
      const deviceMacElement = document.getElementById('deviceMac');
      if (deviceMacElement && data.mac) {
        deviceMacElement.textContent = data.mac;
      }

      // Show/hide Master MAC input based on role
      const masterMacSection = document.getElementById('masterMacSection');
      const macDescription = document.getElementById('macDescription');

      if (data.role == 2) { // Slave role
        masterMacSection.style.display = 'block';
        macDescription.textContent = 'This device\'s MAC address - share with Master device for pairing';

        // Load existing master MAC if available and show connection status
        const masterMacInput = document.getElementById('masterMacInput');
        const connectionStatus = document.getElementById('connectionStatus');
        const connectionStatusText = document.getElementById('connectionStatusText');

        if (data.masterMac && data.masterMac !== '00:00:00:00:00:00') {
          masterMacInput.value = data.masterMac;
          masterMacInput.style.borderColor = '#4cc9f0'; // Show connected state

          // Show connection status
          connectionStatus.style.display = 'block';
          connectionStatus.style.background = '#1a4a4a';
          connectionStatus.style.borderLeft = '4px solid #4cc9f0';
          connectionStatusText.innerHTML = '🟢 Connected to Master: ' + data.masterMac;
          connectionStatusText.style.color = '#4cc9f0';
        } else {
          // Show not connected status
          connectionStatus.style.display = 'block';
          connectionStatus.style.background = '#4a1a1a';
          connectionStatus.style.borderLeft = '4px solid #ff6b6b';
          connectionStatusText.innerHTML = '🔴 Not connected to Master - Enter MAC address above';
          connectionStatusText.style.color = '#ff6b6b';
        }
      } else { // Master role
        masterMacSection.style.display = 'none';
        macDescription.textContent = 'This device\'s MAC address for ESP-NOW pairing';
      }

      // Show/hide real-time section based on role
      const realtimeSection = document.getElementById('realtimeSection');
      if (realtimeSection) {
        if (data.role == 1) { // Master
          realtimeSection.style.display = 'block';
        } else {
          realtimeSection.style.display = 'none';
        }
      }
    })
    .catch(error => console.error('Error loading device info:', error));
}

// Load connected slave devices
function loadSlaveDevices() {
  fetch('/getDeviceInfo')
    .then(r => r.json())
    .then(data => {
      const tbody = document.getElementById('slavesTableBody');
      tbody.innerHTML = '';

      if (data.slaves && data.slaves.length > 0) {
        data.slaves.forEach((mac, index) => {
          const row = createSlaveRow(mac, index);
          tbody.appendChild(row);
        });
      } else {
        tbody.innerHTML = '<tr><td colspan="5" style="text-align:center;color:#666">No slaves connected</td></tr>';
      }
    })
    .catch(error => console.error('Error loading slave devices:', error));
}

// Create slave device table row
function createSlaveRow(mac, index) {
  const row = document.createElement('tr');
  const deviceName = `Slave ${index + 1}`;

  row.innerHTML = `
    <td>${deviceName}</td>
    <td style="font-family:monospace;font-size:10px">${mac}</td>
    <td><span class="signal-medium">Medium</span></td>
    <td><span class="status-active">Connected</span></td>
    <td><button class="btn-small btn-remove" onclick="removeSlave('${mac}')">Remove</button></td>
  `;

  return row;
}

// Scan for AmbiSense devices
function scanForAmbiSenseDevices() {
  if (isScanning) return;

  isScanning = true;
  const scanResults = document.getElementById('scanResults');
  const scanList = document.getElementById('scanList');

  scanResults.style.display = 'block';
  scanList.innerHTML = '<div style="text-align:center;padding:20px;color:#999">🔍 Scanning for AmbiSense devices...</div>';

  fetch('/scanForSlaves')
    .then(r => r.json())
    .then(devices => {
      scanList.innerHTML = '';

      if (devices && devices.length > 0) {
        // Filter for AmbiSense devices only
        const ambiSenseDevices = devices.filter(device =>
          device.name && (device.name.includes('AmbiSense') || device.isAmbiSense)
        );

        if (ambiSenseDevices.length > 0) {
          ambiSenseDevices.forEach(device => {
            const deviceItem = createDeviceItem(device);
            scanList.appendChild(deviceItem);
          });
        } else {
          scanList.innerHTML = '<div style="text-align:center;padding:20px;color:#999">No AmbiSense devices found in range</div>';
        }
      } else {
        scanList.innerHTML = '<div style="text-align:center;padding:20px;color:#999">No devices found</div>';
      }

      isScanning = false;
    })
    .catch(error => {
      console.error('Error scanning for devices:', error);
      scanList.innerHTML = '<div style="text-align:center;padding:20px;color:#ff6b6b">Scan failed. Please try again.</div>';
      isScanning = false;
    });
}

// Create device item for scan results
function createDeviceItem(device) {
  const item = document.createElement('div');
  item.className = 'device-item';

  const signalClass = device.rssi > -50 ? 'signal-strong' :
                     device.rssi > -70 ? 'signal-medium' : 'signal-weak';
  const signalText = device.rssi > -50 ? 'Strong' :
                    device.rssi > -70 ? 'Medium' : 'Weak';

  item.innerHTML = `
    <div>
      <div class="device-name">${device.name || 'AmbiSense Device'}</div>
      <div class="device-mac">${device.mac}</div>
    </div>
    <div>
      <span class="device-signal ${signalClass}">${signalText} (${device.rssi}dBm)</span>
      <button class="btn-small btn-add" onclick="addSlave('${device.mac}')">Add</button>
    </div>
  `;

  return item;
}

// Add slave device
function addSlave(mac) {
  fetch('/addSlave?mac=' + encodeURIComponent(mac))
    .then(r => r.json())
    .then(data => {
      if (data.status === 'success') {
        showSavedNotification();
        loadSlaveDevices();
        // Hide scan results after successful add
        document.getElementById('scanResults').style.display = 'none';
      } else {
        alert('Failed to add device: ' + (data.message || 'Unknown error'));
      }
    })
    .catch(error => {
      console.error('Error adding slave:', error);
      alert('Failed to add device');
    });
}

// Remove slave device
function removeSlave(mac) {
  if (confirm('Remove this slave device?')) {
    fetch('/removeSlave?mac=' + encodeURIComponent(mac))
      .then(r => r.json())
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
          loadSlaveDevices();
        } else {
          alert('Failed to remove device: ' + (data.message || 'Unknown error'));
        }
      })
      .catch(error => {
        console.error('Error removing slave:', error);
        alert('Failed to remove device');
      });
  }
}

// Update real-time sensor data
function updateSensorData() {
  fetch('/sensordata')
    .then(r => r.json())
    .then(data => {
      updateDistanceTable(data);
      updateActiveSensorDisplay(data);
    })
    .catch(error => console.error('Error updating sensor data:', error));
}

// Update distance monitoring table
function updateDistanceTable(data) {
  const tbody = document.getElementById('distanceTableBody');
  if (!tbody) return;

  tbody.innerHTML = '';

  if (data.sensors && data.sensors.length > 0) {
    data.sensors.forEach(sensor => {
      if (sensor.id !== undefined) {
        const row = createDistanceRow(sensor, data.selected);
        tbody.appendChild(row);
      }
    });
  } else {
    tbody.innerHTML = '<tr><td colspan="4" style="text-align:center;color:#666;padding:20px;"><div style="font-size:16px;margin-bottom:8px;">⚪ No Active Sensors</div><div style="font-size:12px;">Pair devices below to see sensor data</div></td></tr>';
  }
}

// Create distance table row
function createDistanceRow(sensor, selectedDistance) {
  const row = document.createElement('tr');
  const age = sensor.age || 0;
  const isActive = age < 5000; // Active if updated within 5 seconds
  const isPriority = sensor.distance == selectedDistance;

  // Generate MAC-like sensor ID
  const sensorId = sensor.id === 0 ? 'Master' : `A${sensor.id}:12:6D:23:...`;

  // Determine signal strength
  let signalClass = 'signal-medium';
  let signalText = 'Medium';
  if (age < 1000) {
    signalClass = 'signal-strong';
    signalText = 'Strong';
  } else if (age > 3000) {
    signalClass = 'signal-weak';
    signalText = 'Weak';
  }

  const distanceText = isActive ? `${sensor.distance} cm` : '--';
  const lastUpdate = `${(age / 1000).toFixed(1)}s ago`;

  row.innerHTML = `
    <td>${sensorId}</td>
    <td>${isPriority ? '<strong>' + distanceText + '</strong>' : distanceText}</td>
    <td><span class="${signalClass}">${signalText}</span></td>
    <td>${lastUpdate}</td>
  `;

  // Highlight active sensor
  if (isPriority) {
    row.className = 'active-sensor';
  }

  return row;
}

// Update active sensor display
function updateActiveSensorDisplay(data) {
  // Find the active sensor display div
  const activeDisplay = document.querySelector('.realtime-monitor > div:last-child > div:last-child');
  if (!activeDisplay) return;

  if (data.selected !== undefined && data.sensors) {
    // Find which sensor is providing the active reading
    let activeSensor = null;
    data.sensors.forEach(sensor => {
      if (sensor.distance == data.selected && sensor.age < 5000) {
        activeSensor = sensor;
      }
    });

    if (activeSensor) {
      const sensorId = activeSensor.id === 0 ? 'Master' : `A${activeSensor.id}:12:6D:23:...`;
      activeDisplay.innerHTML = `
        <div style="font-size:12px;color:#999;">🟢 Active Sensor (driving effect)</div>
        <div style="font-size:16px;font-weight:bold;color:#4cc9f0;">${sensorId} - ${data.selected} cm</div>
      `;
    } else {
      activeDisplay.innerHTML = `
        <div style="font-size:12px;color:#999;">⚪ No Active Sensor</div>
        <div style="font-size:16px;font-weight:bold;color:#666;">-- cm</div>
      `;
    }
  } else {
    activeDisplay.innerHTML = `
      <div style="font-size:12px;color:#999;">⚪ No Active Sensor</div>
      <div style="font-size:16px;font-weight:bold;color:#666;">-- cm</div>
    `;
  }
}

// Load segment configuration
function loadSegmentConfig() {
  fetch('/getLEDSegmentInfo')
    .then(r => r.json())
    .then(data => {
      document.getElementById('ledSegmentMode').value = data.mode || 0;
      document.getElementById('segmentStart').value = data.start || 0;
      document.getElementById('segmentLength').value = data.length || 300;

      // Show/hide segment config based on mode
      const segmentConfig = document.getElementById('segmentConfig');
      if (data.mode == 1) {
        segmentConfig.style.display = 'grid';
      } else {
        segmentConfig.style.display = 'none';
      }
    })
    .catch(error => console.error('Error loading segment config:', error));
}

// Save segment configuration
function saveSegmentConfig() {
  const start = document.getElementById('segmentStart').value;
  const length = document.getElementById('segmentLength').value;
  const total = parseInt(start) + parseInt(length);

  fetch(`/setLEDSegmentInfo?start=${start}&length=${length}&total=${total}`)
    .then(r => r.json())
    .then(data => {
      if (data.status === 'success') {
        showSavedNotification();
      }
    })
    .catch(error => console.error('Error saving segment config:', error));
}

// Add event listeners for segment config
document.addEventListener('DOMContentLoaded', function() {
  const segmentStart = document.getElementById('segmentStart');
  const segmentLength = document.getElementById('segmentLength');

  if (segmentStart) {
    segmentStart.addEventListener('change', saveSegmentConfig);
  }
  if (segmentLength) {
    segmentLength.addEventListener('change', saveSegmentConfig);
  }

  // Initialize mesh tab
  initMeshTab();
});

// Cleanup interval when leaving page
window.addEventListener('beforeunload', function() {
  if (meshUpdateInterval) {
    clearInterval(meshUpdateInterval);
  }
});
</script>
)literal";

const char calibration_tab_full[] PROGMEM = R"literal(
<div class="form-group">
<label>Master Sensor LED Range</label>
<div class="slider-container">
<input type="number" min="0" max="2000" class="led-count-input" id="masterLedStart" value="0">
<span style="color: var(--text-secondary); margin: 0 10px;">to</span>
<input type="number" min="0" max="2000" class="led-count-input" id="masterLedEnd" value="149">
</div>
</div>
<div class="form-group">
<label>Slave Sensor LED Range</label>
<div class="slider-container">
<input type="number" min="0" max="2000" class="led-count-input" id="slaveLedStart" value="150">
<span style="color: var(--text-secondary); margin: 0 10px;">to</span>
<input type="number" min="0" max="2000" class="led-count-input" id="slaveLedEnd" value="299">
</div>
</div>
<button class="button" id="saveCalibrationButton">Save Calibration</button>
)literal";

const char calibration_tab_js[] PROGMEM = R"literal(
<script>
document.getElementById('saveCalibrationButton').addEventListener('click', function() {
    const masterLedStart = document.getElementById('masterLedStart').value;
    const masterLedEnd = document.getElementById('masterLedEnd').value;
    const slaveLedStart = document.getElementById('slaveLedStart').value;
    const slaveLedEnd = document.getElementById('slaveLedEnd').value;
    fetch('/setCalibration?masterStart=' + masterLedStart + '&masterEnd=' + masterLedEnd + '&slaveStart=' + slaveLedStart + '&slaveEnd=' + slaveLedEnd)
        .then(response => response.json())
        .then(data => {
            if (data.status === 'success') {
                showSavedNotification();
            }
        })
        .catch(error => {
            console.error('Error saving calibration:', error);
        });
});

function loadCalibrationSettings() {
    fetch('/getCalibration')
        .then(response => response.json())
        .then(settings => {
            document.getElementById('masterLedStart').value = settings.master_led_start;
            document.getElementById('masterLedEnd').value = settings.master_led_end;
            document.getElementById('slaveLedStart').value = settings.slave_led_start;
            document.getElementById('slaveLedEnd').value = settings.slave_led_end;
        })
        .catch(error => {
            console.error('Error loading calibration settings:', error);
        });
}

window.addEventListener('DOMContentLoaded', (event) => {
    loadCalibrationSettings();
});
</script>
)literal";

// Utility function to build complete HTML page
String buildFullPage(const char* tabContent, const char* activeTab, const __FlashStringHelper* tabScripts = nullptr) {
  String html = FPSTR(html_template_full);

  // Replace CSS and JS
  html.replace("%CSS%", FPSTR(full_css));
  html.replace("%JS%", FPSTR(full_js));

  // Set active tab
  html.replace("%BASIC_TAB_ACTIVE%", strcmp(activeTab, "basic") == 0 ? "active" : "");
  html.replace("%ADVANCED_TAB_ACTIVE%", strcmp(activeTab, "advanced") == 0 ? "active" : "");
  html.replace("%EFFECTS_TAB_ACTIVE%", strcmp(activeTab, "effects") == 0 ? "active" : "");
  html.replace("%MESH_TAB_ACTIVE%", strcmp(activeTab, "mesh") == 0 ? "active" : "");
  html.replace("%CALIBRATION_TAB_ACTIVE%", strcmp(activeTab, "calibration") == 0 ? "active" : "");
  html.replace("%NETWORK_TAB_ACTIVE%", strcmp(activeTab, "network") == 0 ? "active" : "");
  html.replace("%DIAGNOSTICS_TAB_ACTIVE%", strcmp(activeTab, "diagnostics") == 0 ? "active" : "");

  // Insert tab content and scripts
  html.replace("%TAB_CONTENT%", FPSTR(tabContent));
  html.replace("%TAB_SCRIPTS%", tabScripts ? String(tabScripts) : "");

  return html;
}

#endif // COMPRESSED_HTML_FULL_H