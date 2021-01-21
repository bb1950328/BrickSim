/**
 * Fuse.js v6.4.3 - Lightweight fuzzy-search (http://fusejs.io)
 *
 * Copyright (c) 2020 Kiro Risk (http://kiro.me)
 * All Rights Reserved. Apache Software License 2.0
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */
var e,t;e=this,t=function(){"use strict";function e(t){return(e="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function(e){return typeof e}:function(e){return e&&"function"==typeof Symbol&&e.constructor===Symbol&&e!==Symbol.prototype?"symbol":typeof e})(t)}function t(e,t){if(!(e instanceof t))throw new TypeError("Cannot call a class as a function")}function n(e,t){for(var n=0;n<t.length;n++){var r=t[n];r.enumerable=r.enumerable||!1,r.configurable=!0,"value"in r&&(r.writable=!0),Object.defineProperty(e,r.key,r)}}function r(e,t,r){return t&&n(e.prototype,t),r&&n(e,r),e}function i(e,t,n){return t in e?Object.defineProperty(e,t,{value:n,enumerable:!0,configurable:!0,writable:!0}):e[t]=n,e}function o(e,t){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);t&&(r=r.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),n.push.apply(n,r)}return n}function c(e){for(var t=1;t<arguments.length;t++){var n=null!=arguments[t]?arguments[t]:{};t%2?o(Object(n),!0).forEach((function(t){i(e,t,n[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(n)):o(Object(n)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(n,t))}))}return e}function a(e,t){if("function"!=typeof t&&null!==t)throw new TypeError("Super expression must either be null or a function");e.prototype=Object.create(t&&t.prototype,{constructor:{value:e,writable:!0,configurable:!0}}),t&&u(e,t)}function s(e){return(s=Object.setPrototypeOf?Object.getPrototypeOf:function(e){return e.__proto__||Object.getPrototypeOf(e)})(e)}function u(e,t){return(u=Object.setPrototypeOf||function(e,t){return e.__proto__=t,e})(e,t)}function h(e,t){return!t||"object"!=typeof t&&"function"!=typeof t?function(e){if(void 0===e)throw new ReferenceError("this hasn't been initialised - super() hasn't been called");return e}(e):t}function f(e){var t=function(){if("undefined"==typeof Reflect||!Reflect.construct)return!1;if(Reflect.construct.sham)return!1;if("function"==typeof Proxy)return!0;try{return Date.prototype.toString.call(Reflect.construct(Date,[],(function(){}))),!0}catch(e){return!1}}();return function(){var n,r=s(e);if(t){var i=s(this).constructor;n=Reflect.construct(r,arguments,i)}else n=r.apply(this,arguments);return h(this,n)}}function l(e){return function(e){if(Array.isArray(e))return d(e)}(e)||function(e){if("undefined"!=typeof Symbol&&Symbol.iterator in Object(e))return Array.from(e)}(e)||function(e,t){if(e){if("string"==typeof e)return d(e,t);var n=Object.prototype.toString.call(e).slice(8,-1);return"Object"===n&&e.constructor&&(n=e.constructor.name),"Map"===n||"Set"===n?Array.from(e):"Arguments"===n||/^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)?d(e,t):void 0}}(e)||function(){throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.")}()}function d(e,t){(null==t||t>e.length)&&(t=e.length);for(var n=0,r=new Array(t);n<t;n++)r[n]=e[n];return r}function v(e){return Array.isArray?Array.isArray(e):"[object Array]"===x(e)}function g(e){return"string"==typeof e}function y(e){return"number"==typeof e}function p(e){return!0===e||!1===e||function(e){return m(e)&&null!==e}(e)&&"[object Boolean]"==x(e)}function m(t){return"object"===e(t)}function k(e){return null!=e}function M(e){return!e.trim().length}function x(e){return null==e?void 0===e?"[object Undefined]":"[object Null]":Object.prototype.toString.call(e)}var b=function(e){return"Invalid value for key ".concat(e)},L=function(e){return"Pattern length exceeds max of ".concat(e,".")},S=Object.prototype.hasOwnProperty,_=function(){function e(n){var r=this;t(this,e),this._keys=[],this._keyMap={};var i=0;n.forEach((function(e){var t=w(e);i+=t.weight,r._keys.push(t),r._keyMap[t.id]=t,i+=t.weight})),this._keys.forEach((function(e){e.weight/=i}))}return r(e,[{key:"get",value:function(e){return this._keyMap[e]}},{key:"keys",value:function(){return this._keys}},{key:"toJSON",value:function(){return JSON.stringify(this._keys)}}]),e}();function w(e){var t=null,n=null,r=null,i=1;if(g(e)||v(e))r=e,t=O(e),n=j(e);else{if(!S.call(e,"name"))throw new Error(function(e){return"Missing ".concat(e," property in key")}("name"));var o=e.name;if(r=o,S.call(e,"weight")&&(i=e.weight)<=0)throw new Error(function(e){return"Property 'weight' in key '".concat(e,"' must be a positive integer")}(o));t=O(o),n=j(o)}return{path:t,id:n,weight:i,src:r}}function O(e){return v(e)?e:e.split(".")}function j(e){return v(e)?e.join("."):e}var A=c({},{isCaseSensitive:!1,includeScore:!1,keys:[],shouldSort:!0,sortFn:function(e,t){return e.score===t.score?e.idx<t.idx?-1:1:e.score<t.score?-1:1}},{},{includeMatches:!1,findAllMatches:!1,minMatchCharLength:1},{},{location:0,threshold:.6,distance:100},{},{useExtendedSearch:!1,getFn:function(e,t){var n=[],r=!1;return function e(t,i,o){if(k(t))if(i[o]){var c=t[i[o]];if(!k(c))return;if(o===i.length-1&&(g(c)||y(c)||p(c)))n.push(function(e){return null==e?"":function(e){if("string"==typeof e)return e;var t=e+"";return"0"==t&&1/e==-1/0?"-0":t}(e)}(c));else if(v(c)){r=!0;for(var a=0,s=c.length;a<s;a+=1)e(c[a],i,o+1)}else i.length&&e(c,i,o+1)}else n.push(t)}(e,g(t)?t.split("."):t,0),r?n:n[0]},ignoreLocation:!1,ignoreFieldNorm:!1}),I=/[^ ]+/g;function C(){var e=arguments.length>0&&void 0!==arguments[0]?arguments[0]:3,t=new Map;return{get:function(n){var r=n.match(I).length;if(t.has(r))return t.get(r);var i=parseFloat((1/Math.sqrt(r)).toFixed(e));return t.set(r,i),i},clear:function(){t.clear()}}}var E=function(){function e(){var n=arguments.length>0&&void 0!==arguments[0]?arguments[0]:{},r=n.getFn,i=void 0===r?A.getFn:r;t(this,e),this.norm=C(3),this.getFn=i,this.isCreated=!1,this.setIndexRecords()}return r(e,[{key:"setSources",value:function(){var e=arguments.length>0&&void 0!==arguments[0]?arguments[0]:[];this.docs=e}},{key:"setIndexRecords",value:function(){var e=arguments.length>0&&void 0!==arguments[0]?arguments[0]:[];this.records=e}},{key:"setKeys",value:function(){var e=this,t=arguments.length>0&&void 0!==arguments[0]?arguments[0]:[];this.keys=t,this._keysMap={},t.forEach((function(t,n){e._keysMap[t.id]=n}))}},{key:"create",value:function(){var e=this;!this.isCreated&&this.docs.length&&(this.isCreated=!0,g(this.docs[0])?this.docs.forEach((function(t,n){e._addString(t,n)})):this.docs.forEach((function(t,n){e._addObject(t,n)})),this.norm.clear())}},{key:"add",value:function(e){var t=this.size();g(e)?this._addString(e,t):this._addObject(e,t)}},{key:"removeAt",value:function(e){this.records.splice(e,1);for(var t=e,n=this.size();t<n;t+=1)this.records[t].i-=1}},{key:"getValueForItemAtKeyId",value:function(e,t){return e[this._keysMap[t]]}},{key:"size",value:function(){return this.records.length}},{key:"_addString",value:function(e,t){if(k(e)&&!M(e)){var n={v:e,i:t,n:this.norm.get(e)};this.records.push(n)}}},{key:"_addObject",value:function(e,t){var n=this,r={i:t,$:{}};this.keys.forEach((function(t,i){var o=n.getFn(e,t.path);if(k(o))if(v(o))!function(){for(var e=[],t=[{nestedArrIndex:-1,value:o}];t.length;){var c=t.pop(),a=c.nestedArrIndex,s=c.value;if(k(s))if(g(s)&&!M(s)){var u={v:s,i:a,n:n.norm.get(s)};e.push(u)}else v(s)&&s.forEach((function(e,n){t.push({nestedArrIndex:n,value:e})}))}r.$[i]=e}();else if(!M(o)){var c={v:o,n:n.norm.get(o)};r.$[i]=c}})),this.records.push(r)}},{key:"toJSON",value:function(){return{keys:this.keys,records:this.records}}}]),e}();function $(e,t){var n=arguments.length>2&&void 0!==arguments[2]?arguments[2]:{},r=n.getFn,i=void 0===r?A.getFn:r,o=new E({getFn:i});return o.setKeys(e.map(w)),o.setSources(t),o.create(),o}function R(e,t){var n=e.matches;t.matches=[],k(n)&&n.forEach((function(e){if(k(e.indices)&&e.indices.length){var n={indices:e.indices,value:e.value};e.key&&(n.key=e.key.src),e.idx>-1&&(n.refIndex=e.idx),t.matches.push(n)}}))}function F(e,t){t.score=e.score}function P(e){var t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},n=t.errors,r=void 0===n?0:n,i=t.currentLocation,o=void 0===i?0:i,c=t.expectedLocation,a=void 0===c?0:c,s=t.distance,u=void 0===s?A.distance:s,h=t.ignoreLocation,f=void 0===h?A.ignoreLocation:h,l=r/e.length;if(f)return l;var d=Math.abs(a-o);return u?l+d/u:d?1:l}function N(){for(var e=arguments.length>0&&void 0!==arguments[0]?arguments[0]:[],t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:A.minMatchCharLength,n=[],r=-1,i=-1,o=0,c=e.length;o<c;o+=1){var a=e[o];a&&-1===r?r=o:a||-1===r||((i=o-1)-r+1>=t&&n.push([r,i]),r=-1)}return e[o-1]&&o-r>=t&&n.push([r,o-1]),n}function D(e){for(var t={},n=0,r=e.length;n<r;n+=1){var i=e.charAt(n);t[i]=(t[i]||0)|1<<r-n-1}return t}var z=function(){function e(n){var r=this,i=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},o=i.location,c=void 0===o?A.location:o,a=i.threshold,s=void 0===a?A.threshold:a,u=i.distance,h=void 0===u?A.distance:u,f=i.includeMatches,l=void 0===f?A.includeMatches:f,d=i.findAllMatches,v=void 0===d?A.findAllMatches:d,g=i.minMatchCharLength,y=void 0===g?A.minMatchCharLength:g,p=i.isCaseSensitive,m=void 0===p?A.isCaseSensitive:p,k=i.ignoreLocation,M=void 0===k?A.ignoreLocation:k;if(t(this,e),this.options={location:c,threshold:s,distance:h,includeMatches:l,findAllMatches:v,minMatchCharLength:y,isCaseSensitive:m,ignoreLocation:M},this.pattern=m?n:n.toLowerCase(),this.chunks=[],this.pattern.length){var x=function(e,t){r.chunks.push({pattern:e,alphabet:D(e),startIndex:t})},b=this.pattern.length;if(b>32){for(var L=0,S=b%32,_=b-S;L<_;)x(this.pattern.substr(L,32),L),L+=32;if(S){var w=b-32;x(this.pattern.substr(w),w)}}else x(this.pattern,0)}}return r(e,[{key:"searchIn",value:function(e){var t=this.options,n=t.isCaseSensitive,r=t.includeMatches;if(n||(e=e.toLowerCase()),this.pattern===e){var i={isMatch:!0,score:0};return r&&(i.indices=[[0,e.length-1]]),i}var o=this.options,c=o.location,a=o.distance,s=o.threshold,u=o.findAllMatches,h=o.minMatchCharLength,f=o.ignoreLocation,d=[],v=0,g=!1;this.chunks.forEach((function(t){var n=t.pattern,i=t.alphabet,o=t.startIndex,y=function(e,t,n){var r=arguments.length>3&&void 0!==arguments[3]?arguments[3]:{},i=r.location,o=void 0===i?A.location:i,c=r.distance,a=void 0===c?A.distance:c,s=r.threshold,u=void 0===s?A.threshold:s,h=r.findAllMatches,f=void 0===h?A.findAllMatches:h,l=r.minMatchCharLength,d=void 0===l?A.minMatchCharLength:l,v=r.includeMatches,g=void 0===v?A.includeMatches:v,y=r.ignoreLocation,p=void 0===y?A.ignoreLocation:y;if(t.length>32)throw new Error(L(32));for(var m,k=t.length,M=e.length,x=Math.max(0,Math.min(o,M)),b=u,S=x,_=d>1||g,w=_?Array(M):[];(m=e.indexOf(t,S))>-1;){var O=P(t,{currentLocation:m,expectedLocation:x,distance:a,ignoreLocation:p});if(b=Math.min(O,b),S=m+k,_)for(var j=0;j<k;)w[m+j]=1,j+=1}S=-1;for(var I=[],C=1,E=k+M,$=1<<k-1,R=0;R<k;R+=1){for(var F=0,D=E;F<D;){var z=P(t,{errors:R,currentLocation:x+D,expectedLocation:x,distance:a,ignoreLocation:p});z<=b?F=D:E=D,D=Math.floor((E-F)/2+F)}E=D;var K=Math.max(1,x-D+1),q=f?M:Math.min(x+D,M)+k,W=Array(q+2);W[q+1]=(1<<R)-1;for(var J=q;J>=K;J-=1){var T=J-1,U=n[e.charAt(T)];if(_&&(w[T]=+!!U),W[J]=(W[J+1]<<1|1)&U,R&&(W[J]|=(I[J+1]|I[J])<<1|1|I[J+1]),W[J]&$&&(C=P(t,{errors:R,currentLocation:T,expectedLocation:x,distance:a,ignoreLocation:p}))<=b){if(b=C,(S=T)<=x)break;K=Math.max(1,2*x-S)}}var V=P(t,{errors:R+1,currentLocation:x,expectedLocation:x,distance:a,ignoreLocation:p});if(V>b)break;I=W}var B={isMatch:S>=0,score:Math.max(.001,C)};if(_){var G=N(w,d);G.length?g&&(B.indices=G):B.isMatch=!1}return B}(e,n,i,{location:c+o,distance:a,threshold:s,findAllMatches:u,minMatchCharLength:h,includeMatches:r,ignoreLocation:f}),p=y.isMatch,m=y.score,k=y.indices;p&&(g=!0),v+=m,p&&k&&(d=[].concat(l(d),l(k)))}));var y={isMatch:g,score:g?v/this.chunks.length:1};return g&&r&&(y.indices=d),y}}]),e}(),K=function(){function e(n){t(this,e),this.pattern=n}return r(e,[{key:"search",value:function(){}}],[{key:"isMultiMatch",value:function(e){return q(e,this.multiRegex)}},{key:"isSingleMatch",value:function(e){return q(e,this.singleRegex)}}]),e}();function q(e,t){var n=e.match(t);return n?n[1]:null}var W=function(e){a(i,e);var n=f(i);function i(e){return t(this,i),n.call(this,e)}return r(i,[{key:"search",value:function(e){var t=e===this.pattern;return{isMatch:t,score:t?0:1,indices:[0,this.pattern.length-1]}}}],[{key:"type",get:function(){return"exact"}},{key:"multiRegex",get:function(){return/^="(.*)"$/}},{key:"singleRegex",get:function(){return/^=(.*)$/}}]),i}(K),J=function(e){a(i,e);var n=f(i);function i(e){return t(this,i),n.call(this,e)}return r(i,[{key:"search",value:function(e){var t=-1===e.indexOf(this.pattern);return{isMatch:t,score:t?0:1,indices:[0,e.length-1]}}}],[{key:"type",get:function(){return"inverse-exact"}},{key:"multiRegex",get:function(){return/^!"(.*)"$/}},{key:"singleRegex",get:function(){return/^!(.*)$/}}]),i}(K),T=function(e){a(i,e);var n=f(i);function i(e){return t(this,i),n.call(this,e)}return r(i,[{key:"search",value:function(e){var t=e.startsWith(this.pattern);return{isMatch:t,score:t?0:1,indices:[0,this.pattern.length-1]}}}],[{key:"type",get:function(){return"prefix-exact"}},{key:"multiRegex",get:function(){return/^\^"(.*)"$/}},{key:"singleRegex",get:function(){return/^\^(.*)$/}}]),i}(K),U=function(e){a(i,e);var n=f(i);function i(e){return t(this,i),n.call(this,e)}return r(i,[{key:"search",value:function(e){var t=!e.startsWith(this.pattern);return{isMatch:t,score:t?0:1,indices:[0,e.length-1]}}}],[{key:"type",get:function(){return"inverse-prefix-exact"}},{key:"multiRegex",get:function(){return/^!\^"(.*)"$/}},{key:"singleRegex",get:function(){return/^!\^(.*)$/}}]),i}(K),V=function(e){a(i,e);var n=f(i);function i(e){return t(this,i),n.call(this,e)}return r(i,[{key:"search",value:function(e){var t=e.endsWith(this.pattern);return{isMatch:t,score:t?0:1,indices:[e.length-this.pattern.length,e.length-1]}}}],[{key:"type",get:function(){return"suffix-exact"}},{key:"multiRegex",get:function(){return/^"(.*)"\$$/}},{key:"singleRegex",get:function(){return/^(.*)\$$/}}]),i}(K),B=function(e){a(i,e);var n=f(i);function i(e){return t(this,i),n.call(this,e)}return r(i,[{key:"search",value:function(e){var t=!e.endsWith(this.pattern);return{isMatch:t,score:t?0:1,indices:[0,e.length-1]}}}],[{key:"type",get:function(){return"inverse-suffix-exact"}},{key:"multiRegex",get:function(){return/^!"(.*)"\$$/}},{key:"singleRegex",get:function(){return/^!(.*)\$$/}}]),i}(K),G=function(e){a(i,e);var n=f(i);function i(e){var r,o=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},c=o.location,a=void 0===c?A.location:c,s=o.threshold,u=void 0===s?A.threshold:s,h=o.distance,f=void 0===h?A.distance:h,l=o.includeMatches,d=void 0===l?A.includeMatches:l,v=o.findAllMatches,g=void 0===v?A.findAllMatches:v,y=o.minMatchCharLength,p=void 0===y?A.minMatchCharLength:y,m=o.isCaseSensitive,k=void 0===m?A.isCaseSensitive:m,M=o.ignoreLocation,x=void 0===M?A.ignoreLocation:M;return t(this,i),(r=n.call(this,e))._bitapSearch=new z(e,{location:a,threshold:u,distance:f,includeMatches:d,findAllMatches:g,minMatchCharLength:p,isCaseSensitive:k,ignoreLocation:x}),r}return r(i,[{key:"search",value:function(e){return this._bitapSearch.searchIn(e)}}],[{key:"type",get:function(){return"fuzzy"}},{key:"multiRegex",get:function(){return/^"(.*)"$/}},{key:"singleRegex",get:function(){return/^(.*)$/}}]),i}(K),H=function(e){a(i,e);var n=f(i);function i(e){return t(this,i),n.call(this,e)}return r(i,[{key:"search",value:function(e){for(var t,n=0,r=[],i=this.pattern.length;(t=e.indexOf(this.pattern,n))>-1;)n=t+i,r.push([t,n-1]);var o=!!r.length;return{isMatch:o,score:o?1:0,indices:r}}}],[{key:"type",get:function(){return"include"}},{key:"multiRegex",get:function(){return/^'"(.*)"$/}},{key:"singleRegex",get:function(){return/^'(.*)$/}}]),i}(K),Q=[W,H,T,U,B,V,J,G],X=Q.length,Y=/ +(?=([^\"]*\"[^\"]*\")*[^\"]*$)/;function Z(e){var t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{};return e.split("|").map((function(e){for(var n=e.trim().split(Y).filter((function(e){return e&&!!e.trim()})),r=[],i=0,o=n.length;i<o;i+=1){for(var c=n[i],a=!1,s=-1;!a&&++s<X;){var u=Q[s],h=u.isMultiMatch(c);h&&(r.push(new u(h,t)),a=!0)}if(!a)for(s=-1;++s<X;){var f=Q[s],l=f.isSingleMatch(c);if(l){r.push(new f(l,t));break}}}return r}))}var ee=new Set([G.type,H.type]),te=function(){function e(n){var r=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},i=r.isCaseSensitive,o=void 0===i?A.isCaseSensitive:i,c=r.includeMatches,a=void 0===c?A.includeMatches:c,s=r.minMatchCharLength,u=void 0===s?A.minMatchCharLength:s,h=r.ignoreLocation,f=void 0===h?A.ignoreLocation:h,l=r.findAllMatches,d=void 0===l?A.findAllMatches:l,v=r.location,g=void 0===v?A.location:v,y=r.threshold,p=void 0===y?A.threshold:y,m=r.distance,k=void 0===m?A.distance:m;t(this,e),this.query=null,this.options={isCaseSensitive:o,includeMatches:a,minMatchCharLength:u,findAllMatches:d,ignoreLocation:f,location:g,threshold:p,distance:k},this.pattern=o?n:n.toLowerCase(),this.query=Z(this.pattern,this.options)}return r(e,[{key:"searchIn",value:function(e){var t=this.query;if(!t)return{isMatch:!1,score:1};var n=this.options,r=n.includeMatches;e=n.isCaseSensitive?e:e.toLowerCase();for(var i=0,o=[],c=0,a=0,s=t.length;a<s;a+=1){var u=t[a];o.length=0,i=0;for(var h=0,f=u.length;h<f;h+=1){var d=u[h],v=d.search(e),g=v.isMatch,y=v.indices,p=v.score;if(!g){c=0,i=0,o.length=0;break}if(i+=1,c+=p,r){var m=d.constructor.type;ee.has(m)?o=[].concat(l(o),l(y)):o.push(y)}}if(i){var k={isMatch:!0,score:c/i};return r&&(k.indices=o),k}}return{isMatch:!1,score:1}}}],[{key:"condition",value:function(e,t){return t.useExtendedSearch}}]),e}(),ne=[];function re(e,t){for(var n=0,r=ne.length;n<r;n+=1){var i=ne[n];if(i.condition(e,t))return new i(e,t)}return new z(e,t)}var ie="$and",oe="$or",ce="$path",ae="$val",se=function(e){return!(!e[ie]&&!e[oe])},ue=function(e){return!!e[ce]},he=function(e){return!v(e)&&m(e)&&!se(e)},fe=function(e){return i({},ie,Object.keys(e).map((function(t){return i({},t,e[t])})))},le=function(){function e(n){var r=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},i=arguments.length>2?arguments[2]:void 0;t(this,e),this.options=c({},A,{},r),this.options.useExtendedSearch,this._keyStore=new _(this.options.keys),this.setCollection(n,i)}return r(e,[{key:"setCollection",value:function(e,t){if(this._docs=e,t&&!(t instanceof E))throw new Error("Incorrect 'index' type");this._myIndex=t||$(this.options.keys,this._docs,{getFn:this.options.getFn})}},{key:"add",value:function(e){k(e)&&(this._docs.push(e),this._myIndex.add(e))}},{key:"remove",value:function(){for(var e=arguments.length>0&&void 0!==arguments[0]?arguments[0]:function(){return!1},t=[],n=0,r=this._docs.length;n<r;n+=1){var i=this._docs[n];e(i,n)&&(this.removeAt(n),n-=1,r-=1,t.push(i))}return t}},{key:"removeAt",value:function(e){this._docs.splice(e,1),this._myIndex.removeAt(e)}},{key:"getIndex",value:function(){return this._myIndex}},{key:"search",value:function(e){var t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},n=t.limit,r=void 0===n?-1:n,i=this.options,o=i.includeMatches,c=i.includeScore,a=i.shouldSort,s=i.sortFn,u=i.ignoreFieldNorm,h=g(e)?g(this._docs[0])?this._searchStringList(e):this._searchObjectList(e):this._searchLogical(e);return de(h,{ignoreFieldNorm:u}),a&&h.sort(s),y(r)&&r>-1&&(h=h.slice(0,r)),ve(h,this._docs,{includeMatches:o,includeScore:c})}},{key:"_searchStringList",value:function(e){var t=re(e,this.options),n=this._myIndex.records,r=[];return n.forEach((function(e){var n=e.v,i=e.i,o=e.n;if(k(n)){var c=t.searchIn(n),a=c.isMatch,s=c.score,u=c.indices;a&&r.push({item:n,idx:i,matches:[{score:s,value:n,norm:o,indices:u}]})}})),r}},{key:"_searchLogical",value:function(e){var t=this,n=function(e,t){var n=arguments.length>2&&void 0!==arguments[2]?arguments[2]:{},r=n.auto,i=void 0===r||r,o=function e(n){var r=Object.keys(n),o=ue(n);if(!o&&r.length>1&&!se(n))return e(fe(n));if(he(n)){var c=o?n[ce]:r[0],a=o?n[ae]:n[c];if(!g(a))throw new Error(b(c));var s={keyId:j(c),pattern:a};return i&&(s.searcher=re(a,t)),s}var u={children:[],operator:r[0]};return r.forEach((function(t){var r=n[t];v(r)&&r.forEach((function(t){u.children.push(e(t))}))})),u};return se(e)||(e=fe(e)),o(e)}(e,this.options),r=this._myIndex.records,i={},o=[];return r.forEach((function(e){var r=e.$,c=e.i;if(k(r)){var a=function e(n,r,i){if(!n.children){var o=n.keyId,c=n.searcher,a=t._findMatches({key:t._keyStore.get(o),value:t._myIndex.getValueForItemAtKeyId(r,o),searcher:c});return a&&a.length?[{idx:i,item:r,matches:a}]:[]}switch(n.operator){case ie:for(var s=[],u=0,h=n.children.length;u<h;u+=1){var f=e(n.children[u],r,i);if(!f.length)return[];s.push.apply(s,l(f))}return s;case oe:for(var d=[],v=0,g=n.children.length;v<g;v+=1){var y=e(n.children[v],r,i);if(y.length){d.push.apply(d,l(y));break}}return d}}(n,r,c);a.length&&(i[c]||(i[c]={idx:c,item:r,matches:[]},o.push(i[c])),a.forEach((function(e){var t,n=e.matches;(t=i[c].matches).push.apply(t,l(n))})))}})),o}},{key:"_searchObjectList",value:function(e){var t=this,n=re(e,this.options),r=this._myIndex,i=r.keys,o=r.records,c=[];return o.forEach((function(e){var r=e.$,o=e.i;if(k(r)){var a=[];i.forEach((function(e,i){a.push.apply(a,l(t._findMatches({key:e,value:r[i],searcher:n})))})),a.length&&c.push({idx:o,item:r,matches:a})}})),c}},{key:"_findMatches",value:function(e){var t=e.key,n=e.value,r=e.searcher;if(!k(n))return[];var i=[];if(v(n))n.forEach((function(e){var n=e.v,o=e.i,c=e.n;if(k(n)){var a=r.searchIn(n),s=a.isMatch,u=a.score,h=a.indices;s&&i.push({score:u,key:t,value:n,idx:o,norm:c,indices:h})}}));else{var o=n.v,c=n.n,a=r.searchIn(o),s=a.isMatch,u=a.score,h=a.indices;s&&i.push({score:u,key:t,value:o,norm:c,indices:h})}return i}}]),e}();function de(e,t){var n=t.ignoreFieldNorm,r=void 0===n?A.ignoreFieldNorm:n;e.forEach((function(e){var t=1;e.matches.forEach((function(e){var n=e.key,i=e.norm,o=e.score,c=n?n.weight:null;t*=Math.pow(0===o&&c?Number.EPSILON:o,(c||1)*(r?1:i))})),e.score=t}))}function ve(e,t){var n=arguments.length>2&&void 0!==arguments[2]?arguments[2]:{},r=n.includeMatches,i=void 0===r?A.includeMatches:r,o=n.includeScore,c=void 0===o?A.includeScore:o,a=[];return i&&a.push(R),c&&a.push(F),e.map((function(e){var n=e.idx,r={item:t[n],refIndex:n};return a.length&&a.forEach((function(t){t(e,r)})),r}))}return le.version="6.4.3",le.createIndex=$,le.parseIndex=function(e){var t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},n=t.getFn,r=void 0===n?A.getFn:n,i=e.keys,o=e.records,c=new E({getFn:r});return c.setKeys(i),c.setIndexRecords(o),c},le.config=A,function(){ne.push.apply(ne,arguments)}(te),le},"object"==typeof exports&&"undefined"!=typeof module?module.exports=t():"function"==typeof define&&define.amd?define(t):(e=e||self).Fuse=t();
;
const idx = [
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/installation/",
    "title": "Installation",
    "body": `[The installation process is different on each operating system. Windows Linux Mac OS X]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/features/planned/",
    "title": "Planned Features",
    "body": `[The following table lists all bigger features which are planned but not realized yet. Feel free to vote for them on their GitHub Issue. Description GitHub Issue Display a transform gizmo to translate and rotate things directly inside the 3D view #2 Save the model back to a .ldr or .mpd file #3 Pan function in 3D view #4 Customizable line thickness #5 Set camera origin to part location #7 Static analysis (just here for completeness, there's a separate page for this) Milestone #1 More sophisticated part search to handle queries like title=Hello OR name=World #8 Universal keymap (user can configure keyboard shortcut or mouse event for all actions #11]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/technical_info/technologies_dependencies/",
    "title": "Technologies and dependencies",
    "body": `[Logo Description BrickSim uses C++. The main reason for this choice was execution speed. BrickSim is built using the platform-independent build system CMake. The User Interface is made with Dear ImGui. OpenGL is used for rendering on all platforms. SQLite3 is used to save settings and cache to the disk. The library SQLiteCpp is used. Hugo is a static site generator using go. It is used in BrickSimWeb. mingw-std-threads is needed to use std::thread on MinGW. GLFW is used to create a window and handle inputs. glad is used to load the OpenGL library glew is used to load OpenGL extensions. Miniball is used to determine the smallest enclosing ball of points, for example to center the part thumbnails. stb_image.h and stb_image_write.h and is used for image reading and writing. tinyfiladialogs is used to open native file dialogs on all platforms.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/installation/windows/",
    "title": "Windows",
    "body": `[currently, the only way to run BrickSim on Windows is to build it from source. A .exe file will be provided as soon as I have figured out how to build it in GitHub Actions on Windows.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/contributing/code/workspace_setup/",
    "title": "Workspace Setup",
    "body": `[This page describes how to set up your workspace, so you can develop on BrickSim too. Preparation on Windows Download MSYS2 from https://www.msys2.org/ and follow the installation instructions there. Execute the following command in a MSYS2 Shell to install git: pacman -S git Clone the repository by git clone --recurse-submodules -j8 git://github.com/bb1950328/BrickSim.git Preparation on MacOS Execute the following command to install the XCode command line tools if you haven't already: sudo xcode-select --install Setup script After you have done the needed preparations, you can execute the setup_workspace.sh Open the IDE The project is CMake-based, so your IDE should support CMake. I recommend you to use CLion. A tutorial for VS Code (#15), Visual Studio 2019 (#16) and XCode (#16) is in planning.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/contributing/code/",
    "title": "Code",
    "body": `[If you want to contribute to the code, you should know the following things: How git and GitHub works How to code in C++ How Dear ImGui works (The Demo is included in BrickSim, the best way for me was to play around and then Ctrl+F for a display text to quickly navigate to the corresponding code lines) If you don't know one of the technologies yet, there are many tutorials on the internet. If you have an idea or a bug, please make a new issue and describe it as detailed as possible. Also mention that you want to implement it yourself. The maintainers will give you their OK if the issue is valid. After that you can start coding. This way you don't waste time for something which we won't accept. Of course, we accept as much as possible, but if something does not make sense, it is better not to accept it. Before you start, make sure you read the technical documentation.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/technical_info/code_structure/",
    "title": "Code structure",
    "body": `[All source files are located in the src/ Subfolder. The src/lib/ folder is for all third-party code. Files Purpose Description main.cpp Starter This file is the entry point and just creates the controller. controller.h controller.cpp Controller The controller controls the program flow. shaders/* Shaders This folder contains a Shader class which is used to load shaders and modify their state. The other files are the shaders itself, written in the GLSL language. helpers/camera.h helpers/camera.cpp Camera object helpers/util.h helpers/util.cpp Utils This files contain various utility functions. helpers/zip_buffer.h helpers/zip_buffer.cpp .zip buffer Can load a .zip file into memory. You can then get the file content as std::string or std::stringstream helpers/part_color_availability_provider.h helpers/part_color_availability_provider.cpp helpers/price_guide_provider.h helpers/price_guide_provider.cpp BrickLink info providers Load information about parts from www.bricklink.com gui/gui.h gui/gui.cpp gui/gui_internal.h gui/gui_internal.cpp GUI Central GUI functionality gui/window_*.cpp GUI windows There should be one file per window. If you add a new file, you should also add it in gui::drawMainWindows::windowFuncsAndState. config.h config.cpp Configuration Get and set user preferences. db.h db.cpp Database Access to the SQLite3 databases element_tree.h element_tree.cpp Element Tree The element tree is the main data structure of BrickSim. It consists of different types of nodes. ldr_colors.h ldr_colors.cpp LDraw Color class for LDraw color definition ldr_file_repository.h ldr_file_repository.cpp LDraw File Repository All LDraw files are opened through this. It manages different types of part libraries (folder/zip) and caches the opened files. ldr_files.h ldr_files.cpp LDraw File The LDraw file format is implemented here mesh.h mesh.cpp Mesh All objects are displayed in 3D with a mesh. It can have multiple instances in multiple colors. It also contains the functionality to allocate buffers on the VRAM and draws the object using these buffers. mesh_collection.h mesh_collection.cpp Mesh organisation The mesh collection organizes the meshes. orientation_cube.h orientation_cube.cpp Orientation Cube These two files are used to render the orientation cube, a cube which shows the camera orientation. part_finder.h part_finder.cpp Part finder As the name says, this provides the part search functionality. renderer.h renderer.cpp Renderer This part of the program renders the 3D View. thumbnail_generator.h thumbnail_generator.cpp Thumbnail Generator The thumbnails in the part palette window are rendered in these files.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/installation/linux/",
    "title": "Linux",
    "body": `[Go to CMake Linux Build, select the topmost run and download the &quot;BrickSimLinux&quot; artifact. Unzip it somewhere Open the terminal and go to the folder you unzipped the files Install the package using sudo apt install ./BrickSim.deb Maybe you will get a message like E: Unmet dependencies. Try 'apt --fix-broken install' with no packages (or specify a solution). As the message already says, you have to execute apt --fix-broken install Run the program with BrickSim If you want to uninstall it, execute sudo apt remove bricksim]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/features/static_analysis/",
    "title": "Static Analysis",
    "body": `[Vision Imagine designing a large bridge with LEGO(R) parts. Wouldn't it be useful if you could see whether your construction is robust before you order the parts? Depending on that, you might even find a better solution that is stronger, uses fewer parts, or looks better. All that should be possible - if you have the right software. Unfortunately, there's currently no software capable of doing a physics analysis of a LEGO model. BrickSim will change that. Plan / Ideas The analysis model is just another object inside the element tree. The user can hide it delete it enable automatic recomputing if his computer is fast enough or his model small enough If the element is visible, the bricks should be displayed in rainbow colors over the original bricks (red means high stress, green means low stress) Technical Implementation Part weight The weight of all parts that consist only of normal ABS plastic can be calculated relatively easily. We can calculate the volume using the method described in this paper or use a library like the GNU Triangulated Surface Library. We can get the weights of each brick directly using the BrickLink Catalog Download. This is the only way to get the correct volume for all parts which contain non-ABS materials. Connections between parts A lot of information about how the parts can be connected is already available in the LDraw parts library. All parts are composed of primitives. The Primitive Reference lists them all. Unfortunately not all connections can be derived from primitives. For example the underside stud of a 1x1 brick isn't made from primitives. Therefore, we need a list to add additional connections. This component can later be reused for part snapping. Data Structure It's better to not use the element tree directly as the data structure. It should be simplified to a collection of parts because physics do not care about the part hierarchy of a model, and the code would just be unnecessarily complex.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/technical_info/",
    "title": "Technical Information",
    "body": `[Read this section if you are interested how BrickSim is made.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/contributing/code/vscode_setup_windows/",
    "title": "Using VS Code on Windows",
    "body": `[add C:\msys64\mingw64\bin to PATH Download and install VS Code from here if you haven't already Install the extensions ms-vscode.cpptools and ms-vscode.cmake-tools Open the BrickSim folder in VS Code. you should get the notifications like in the following screenshot (at least the first one): click &quot;yes&quot; on the first notification, we'll deal with the second one later. Now it's time to adjust some settings: Press Ctrl + Shift+ P and type &quot;preferences: Open Settings (JSON)&quot; Hit Enter or click the first result. add the following text: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 { &#34;cmake.configureOnOpen&#34;: true, &#34;cmake.cmakePath&#34;: &#34;C:\\msys64\\mingw64\\bin\\cmake.exe&#34;, &#34;cmake.generator&#34;: &#34;MinGW Makefiles&#34;, &#34;cmake.buildDirectory&#34;: &#34;${workspaceFolder}/cmake-build&#34;, &#34;cmake.mingwSearchDirs&#34;: [ &#34;C:\\msys64\\mingw64\\bin&#34; ], &#34;cmake.debugConfig&#34;: { &#34;cwd&#34;: &#34;${workspaceFolder}&#34; }, &#34;terminal.integrated.shell.windows&#34;: &#34;C:\\msys64\\usr\\bin\\bash.exe&#34;, &#34;terminal.integrated.shellArgs.windows&#34;: [&#34;-li&#34;], &#34;terminal.integrated.env.windows&#34;: { &#34;MSYSTEM&#34;: &#34;MINGW64&#34;, &#34;CHERE_INVOKING&#34;: &#34;1&#34;, }, } Maybe you already have some settings here so make sure they don't contradict each other. Save the settings.json file. Now press Ctrl+Shift+P and type &quot;CMake: Edit User-Local CMake Kits&quot; Insert the following code: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 [ { &#34;name&#34;: &#34;Mingw64 GCC&#34;, &#34;compilers&#34;: { &#34;C&#34;: &#34;C:\\msys64\\mingw64\\bin\\gcc.exe&#34;, &#34;CXX&#34;: &#34;C:\\msys64\\mingw64\\bin\\g++.exe&#34; }, &#34;preferredGenerator&#34;: { &#34;name&#34;: &#34;MinGW Makefiles&#34;, &#34;platform&#34;: &#34;x64&#34; }, &#34;environmentVariables&#34;: { &#34;PATH&#34;: &#34;C:/msys64/mingw64/bin/&#34; } } ] Save the cmake-tools-kits.json file. Download git for windows from here and install it if you want git integration in VS Code. Restart VS Code You should now be able to launch the application using the bottom status bar:]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/contributing/web/",
    "title": "Website",
    "body": `[sdafasdasdf]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/contributing/code_of_conduct/",
    "title": "Code of Conduct",
    "body": `[Contributor Covenant Code of Conduct Our Pledge We as members, contributors, and leaders pledge to make participation in our community a harassment-free experience for everyone, regardless of age, body size, visible or invisible disability, ethnicity, sex characteristics, gender identity and expression, level of experience, education, socio-economic status, nationality, personal appearance, race, religion, or sexual identity and orientation. We pledge to act and interact in ways that contribute to an open, welcoming, diverse, inclusive, and healthy community. Our Standards Examples of behavior that contributes to a positive environment for our community include: Demonstrating empathy and kindness toward other people Being respectful of differing opinions, viewpoints, and experiences Giving and gracefully accepting constructive feedback Accepting responsibility and apologizing to those affected by our mistakes, and learning from the experience Focusing on what is best not just for us as individuals, but for the overall community. Examples of unacceptable behavior include: The use of sexualized language or imagery, and sexual attention or advances of any kind Trolling, insulting or derogatory comments, and personal or political attacks Public or private harassment Publishing others' private information, such as a physical or email address, without their explicit permission Submitting low-effort pull requests for an event like Hacktoberfest. You will waste your own time as well as the time of the maintainers so just don't do it. Other conduct which could reasonably be considered inappropriate in a professional setting Enforcement Responsibilities Community leaders are responsible for clarifying and enforcing our standards of acceptable behavior and will take appropriate and fair corrective action in response to any behavior that they deem inappropriate, threatening, offensive, or harmful. Community leaders have the right and responsibility to remove, edit, or reject comments, commits, code, wiki edits, issues, and other contributions that are not aligned to this Code of Conduct, and will communicate reasons for moderation decisions when appropriate. Scope This Code of Conduct applies within all community spaces, and also applies when an individual is officially representing the community in public spaces. Examples of representing our community include using an official e-mail address, posting via an official social media account, or acting as an appointed representative at an online or offline event. Enforcement Instances of abusive, harassing, or otherwise unacceptable behavior may be reported to the community leaders responsible for enforcement by tagging @bb1950328 if it's happening on GitHub Issues or otherwise by writing an e-mail to bb1950328@gmail.com. All complaints will be reviewed and investigated promptly and fairly. All community leaders are obligated to respect the privacy and security of the reporter of any incident. Enforcement Guidelines Community leaders will follow these Community Impact Guidelines in determining the consequences for any action they deem in violation of this Code of Conduct: 1. Correction Community Impact: Use of inappropriate language or other behavior deemed unprofessional or unwelcome in the community. Consequence: A private, written warning from community leaders, providing clarity around the nature of the violation and an explanation of why the behavior was inappropriate. A public apology may be requested. 2. Warning Community Impact: A violation through a single incident or series of actions. Consequence: A warning with consequences for continued behavior. No interaction with the people involved, including unsolicited interaction with those enforcing the Code of Conduct, for a specified period of time. This includes avoiding interactions in community spaces as well as external channels like social media. Violating these terms may lead to a temporary or permanent ban. 3. Temporary Ban Community Impact: A serious violation of community standards, including sustained inappropriate behavior. Consequence: A temporary ban from any sort of interaction or public communication with the community for a specified period of time. No public or private interaction with the people involved, including unsolicited interaction with those enforcing the Code of Conduct, is allowed during this period. Violating these terms may lead to a permanent ban. 4. Permanent Ban Community Impact: Demonstrating a pattern of violation of community standards, including sustained inappropriate behavior, harassment of an individual, or aggression toward or disparagement of classes of individuals. Consequence: A permanent ban from any sort of public interaction within the community. Attribution This Code of Conduct is adapted from the Contributor Covenant, version 2.0, available at https://www.contributor-covenant.org/version/2/0/code_of_conduct.html. Community Impact Guidelines were inspired by Mozilla's code of conduct enforcement ladder. For answers to common questions about this code of conduct, see the FAQ at https://www.contributor-covenant.org/faq. Translations are available at https://www.contributor-covenant.org/translations.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/contributing/",
    "title": "Contributing",
    "body": `[GitHub Repositories BrickSim is an open-source project. This means that everyone can help improve the program. All files are on GitHub, divided into two repositories. The main repo https://github.com/bb1950328/BrickSim/ contains all files for the program itself and a docs/ folder which has contains the project website. This folder is not meant to be edited by hand because the source files are in another repo (BrickSimWeb). Of course, it would be better if the docs/ folder was in the BrickSimWeb repository too, but the URL for GitHub Pages is the same as the repository name. This GitHub Action Workflow synchronizes all changes from BrickSimWeb to BrickSim/docs. If you want to contribute to the website (writing manuals etc.), you need to clone BrickSimWeb. More about contributing to the website can be found here. If you want to contribute to the program itself, you need to clone BrickSim. GitHub Issue Tracker All issues that only have something to do with the documentation or the website should go on BrickSimWeb/issues. Everything else goes to BrickSim/issues. If you have found a bug, have a question or feature request, check if there's already something in the docs about it. If there's nothing about it in the docs, search on GitHub Issues. In case you still don't have a satisfactory answer, don't hesitate to create a new issue. Security Issues Every program has bigger or smaller bugs. Unfortunately, some of them are security vulnerabilities. It is safer for all users if these are not publicly reported. If you found one, please write an e-mail directly to the creator of BrickSim (bb1950328@gmail.com).]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/technical_info/documents/",
    "title": "External documents",
    "body": `[OpenGL Khronos OpenGL Wiki OpenGL 3.3 Core Specification OpenGL Tutorial (Also available as PDF) LDraw LDraw Homepage File Format Specification !COLOUR Specification Back Face Culling Specification MPD Specification]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/installation/osx/",
    "title": "Mac OS X",
    "body": `[currently, the only way to run BrickSim a Mac is to build it from source. A binary file will be provided as soon as I have found someone who knows how to build software on Mac OS X.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/features/",
    "title": "Features",
    "body": `[This section gives you more information about the features of BrickSim.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/installation/get_codes_txt/",
    "title": "How to get the codes.txt file",
    "body": `[The codes.txt file contains all parts with the colors they are available in. Because it's unclear whether it's allowed to distribute that file together with the program, you have to download it manually. It's really simple: Go to https://www.bricklink.com/catalogDownload.asp Select &quot;Part and Color Codes&quot;. Also make sure &quot;Tab-Delimited File&quot; is selected. Click on the download button and save the file in the BrickSim folder]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/",
    "title": "BrickSim",
    "body": `[This Program is currently under development and shouldn't be used in a productive environment yet. If you want to help, look at the contribution section in the docs. BrickSim lorem ipsum Read the Docs Look at the screenshots]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/contributing/code/workspace_setup_macos/",
    "title": "",
    "body": `[]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/screenshots/",
    "title": "",
    "body": `[Here you can see how BrickSim looks and what the main features are.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/categories/",
    "title": "Categories",
    "body": `[]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/docs/",
    "title": "Docs",
    "body": `[All Knowledge about BrickSim is placed inside this Section.]`.toLowerCase(),
  },
  {
    "link": "https://bb1950328.github.io/BrickSim/tags/",
    "title": "Tags",
    "body": `[]`.toLowerCase(),
  },
];

const searchKeys = ['title', 'link', 'body', 'id'];

const searchPageElement = elem('#searchpage');

const searchOptions = {
  ignoreLocation: true,
  findAllMatches: true,
  includeScore: true,
  shouldSort: true,
  keys: searchKeys,
  threshold: 0.0
};

const index = new Fuse(idx, searchOptions);

function minQueryLen(query) {
  query = query.trim();
  const queryIsFloat = parseFloat(query);
  const minimumQueryLength = queryIsFloat ? 1 : 2;
  console.log(query, queryIsFloat, minimumQueryLength);
  return minimumQueryLength;
}

function searchResults(results=[], query="", passive = false) {
  let resultsFragment = new DocumentFragment();
  let showResults = elem('.search_results');
  if(passive) {
    showResults = searchPageElement;
  }
  emptyEl(showResults);

  const queryLen = query.length;
  const requiredQueryLen = minQueryLen(query);

  if(results.length && queryLen >= requiredQueryLen) {
    console.log('hmm');
    let resultsTitle = createEl('h3');
    resultsTitle.className = 'search_title';
    resultsTitle.innerText = quickLinks;
    if(passive) {
      resultsTitle.innerText = searchResultsLabel;
    }
    resultsFragment.appendChild(resultsTitle);
    if(!searchPageElement) {
      results = results.slice(0,8);
    } else {
      results = results.slice(0,12);
    }
    results.forEach(function(result){
      let item = createEl('a');
      item.href = `${result.link}?query=${query}`;
      item.className = 'search_result';
      item.style.order = result.score;
      if(passive) {
        pushClass(item, 'passive');
        let itemTitle = createEl('h3');
        itemTitle.textContent = result.title;
        item.appendChild(itemTitle);

        let itemDescription = createEl('p');
        // position of first search term instance
        let queryInstance = result.body.indexOf(query);
        itemDescription.textContent = `... ${result.body.substring(queryInstance, queryInstance + 200)} ...`;
        item.appendChild(itemDescription);
      } else {
        item.textContent = result.title;
      }
      resultsFragment.appendChild(item);
    });
  }

  if(queryLen >= requiredQueryLen) {
    if (!results.length) {
      showResults.innerHTML = `<span class="search_result">${noMatchesFound}</span>`;
    }
  } else {
    if (queryLen > 1) {
      showResults.innerHTML = `<label for="find" class="search_result">${shortSearchQuery}</label>`;
    } else {
      showResults.innerHTML = `<label for="find" class="search_result">${typeToSearch}</label>`;
    }
  }

  showResults.appendChild(resultsFragment);
}

function search(searchTerm, passive = false) {
  if(searchTerm.length) {
    let rawResults = index.search(searchTerm);
    rawResults = rawResults.map(function(result){
      const score = result.score;
      const resultItem = result.item;
      resultItem.score = (parseFloat(score) * 50).toFixed(0);
      return resultItem;
    });

    passive ? searchResults(rawResults, searchTerm, true) : searchResults(rawResults, searchTerm);

  } else {
    passive ? searchResults([], "", true) : searchResults();
  }
}

function liveSearch() {
  const searchField = elem('.search_field');

  if (searchField) {
    searchField.addEventListener('input', function() {
      const searchTerm = searchField.value.trim().toLowerCase();
      search(searchTerm);
    });

    if(!searchPageElement) {
      searchField.addEventListener('search', function(){
        const searchTerm = searchField.value.trim().toLowerCase();
        if(searchTerm.length)  {
          window.location.href = `${parentURL}search/?query=${searchTerm}`;
        }
      });
    }
  }
}

function findQuery(query = 'query') {
  const urlParams = new URLSearchParams(window.location.search);
  if(urlParams.has(query)){
    let c = urlParams.get(query);
    return c;
  }
  return "";
}

function passiveSearch() {
  if(searchPageElement) {
    const searchTerm = findQuery();
    search(searchTerm, true);

    // search actively after search page has loaded
    const searchField = elem('.search_field');

    if(searchField) {
      searchField.addEventListener('input', function() {
        const searchTerm = searchField.value.trim().toLowerCase();
        search(searchTerm, true);
        wrapText(searchTerm, main);
      });
    }
  }
}

function hasSearchResults() {
  const searchResults = elem('.results');
  const body = searchResults.innerHTML.length;
  return [searchResults, body]
}

function clearSearchResults() {
  let searchResults = hasSearchResults();
  let actionable = searchResults[1];
  if(actionable) {
    searchResults = searchResults[0];
    searchResults.innerHTML = "";
    // clear search field
    const searchField = elem('.search_field');
    searchField.value = "";
  }
}

function onEscape(fn){
  window.addEventListener('keydown', function(event){
    if(event.code === "Escape") {
      fn();
    }
  });
}

let main = elem('main');
if(!main) {
  main = elem('.main');
}

window.addEventListener('load', function() {
  searchPageElement ? false : liveSearch();
  passiveSearch();

  wrapText(findQuery(), main);

  onEscape(clearSearchResults);
});

window.addEventListener('click', function(event){
  const target = event.target;
  const isSearch = target.closest('.search') || target.matches('.search');
  if(!isSearch && !searchPageElement) {
    clearSearchResults();
  }
});
