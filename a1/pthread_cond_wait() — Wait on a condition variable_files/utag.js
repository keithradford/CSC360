//tealium universal tag - utag.178 ut4.0.202008241237, Copyright 2020 Tealium.com Inc. All Rights Reserved.
try{(function(id,loader){var u={};utag.o[loader].sender[id]=u;if(utag===undefined){utag={};}if(utag.ut===undefined){utag.ut={};}if(utag.ut.loader===undefined){u.loader=function(o){var a,b,c,l;a=document;if(o.type==="iframe"){b=a.createElement("iframe");b.setAttribute("height","1");b.setAttribute("width","1");b.setAttribute("style","display:none");b.setAttribute("src",o.src);}else if(o.type==="img"){utag.DB("Attach img: "+o.src);b=new Image();b.src=o.src;return;}else{b=a.createElement("script");b.language="javascript";b.type="text/javascript";b.async=1;b.charset="utf-8";b.src=o.src;}if(o.id){b.id=o.id;}if(typeof o.cb==="function"){if(b.addEventListener){b.addEventListener("load",function(){o.cb();},false);}else{b.onreadystatechange=function(){if(this.readyState==="complete"||this.readyState==="loaded"){this.onreadystatechange=null;o.cb();}};}}l=o.loc||"head";c=a.getElementsByTagName(l)[0];if(c){utag.DB("Attach to "+l+": "+o.src);if(l==="script"){c.parentNode.insertBefore(b,c);}else{c.appendChild(b);}}};}else{u.loader=utag.ut.loader;}
u.ev={'view':1};u.initialized=false;u.map={};u.extend=[function(a,b){try{if(1){try{var MM_UUID='';window.mmUuidSync=function(json){MM_UUID=json.uuid||'';if(/^[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}$/.test(MM_UUID)&&!!window.ibmStats&&!!ibmStats.event&&!_appInfo.dbdmDryRun){var eventInfo={type:'element',primaryCategory:'BEACON-SYNC',eventName:'MEDIAMATH',eventCallBackCode:MM_UUID};ibmStats.event(eventInfo);}}}catch(error){_dl.fn.log.add('+++DBDM-ERROR > Opal_MediaMath_Sync_Pixel_for_GA_post_processing.js: '+error);_dl.fn.page.sendBeacon("ERROR > Opal_MediaMath_Sync_Pixel_for_GA_post_processing.js","Error: "+error);}}}catch(e){utag.DB(e)}},function(a,b){try{if(typeof b['cp.utag_main_mm_ga_sync']=='undefined'){utag.loader.SC('utag_main',{'mm_ga_sync':'1'+';exp-session'});b['cp.utag_main_mm_ga_sync']='1';}}catch(e){utag.DB(e);}}];u.send=function(a,b){if(u.ev[a]||u.ev.all!==undefined){var c,d,e,f,i;u.data={"base_url":"//pixel.mathtag.com/sync/js?cs_jsonp=mmUuidSync&mt_nsync=1"
};for(c=0;c<u.extend.length;c++){try{d=u.extend[c](a,b);if(d==false)return}catch(e){if(typeof utag_err!='undefined'){utag_err.push({e:'extension error:'+e,s:utag.cfg.path+'utag.'+id+'.js',l:c,t:'ex'})}}};for(d in utag.loader.GV(u.map)){if(b[d]!==undefined&&b[d]!==""){e=u.map[d].split(",");for(f=0;f<e.length;f++){u.data[e[f]]=b[d];}}}
u.loader_cb=function(){};if(!u.initialized){u.loader({"type":"script","src":u.data.base_url,"cb":u.loader_cb,"loc":"script","id":'utag_178'});}else{u.loader_cb();}
}};utag.o[loader].loader.LOAD(id);})("178","ibm.web");}catch(error){utag.DB(error);}
