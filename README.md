Nginx Module for Google
=======================

#### Description ####
`ngx_http_google_filter_module` is a filter module which makes google mirror much easier to deploy.    
The regular expression, uri locations and other complex configurations have been built-in already.    
The native nginx module ensure the efficiency of handling cookies, gstatic scoures and redirections.   
Let's see how `easy` it is to setup a google mirror.
```nginx
location / {
  google on;
}
```
> _What? Are you kidding me?_   
> _Yes, it's just that simple!_
  
#### Demo site [https://wen.lu](https://wen.lu) ####
![Demo Site](http://ww4.sinaimg.cn/large/68bd1777jw1enbhxn39z8j212q0lu0uo.jpg)
  
#### Dependency ####
  1. [`pcre`](http://www.pcre.org/) *regular expression support*
  1. [`ngx_http_proxy_module`](http://nginx.org/en/docs/http/ngx_http_proxy_module.html) *backend proxy support*
  1. [`ngx_http_substitutions_filter_module`](https://github.com/yaoweibin/ngx_http_substitutions_filter_module) *mutiple substitutions support*

#### Installation ####
##### Download sources first #####
```
#
# download the newest source
# @see http://nginx.org/en/download.html
#
wget http://nginx.org/download/nginx-1.7.8.tar.gz

#
# clone ngx_http_google_filter_module
# @see https://github.com/cuber/ngx_http_google_filter_module
#
git clone https://github.com/cuber/ngx_http_google_filter_module

#
# clone ngx_http_substitutions_filter_module
# @see https://github.com/yaoweibin/ngx_http_substitutions_filter_module
#
git clone https://github.com/yaoweibin/ngx_http_substitutions_filter_module
```
##### Brand new installation #####
``` bash
#
# configure nginx customly
# replace </path/to/> to your real path
#
./configure \
  <your configuration> \
  --add-module=</path/to/>ngx_http_google_filter_module \
  --add-module=</path/to/>ngx_http_substitutions_filter_module
```

##### Migrate from existed distribution #####
```bash
#
# download the same version of nginx source
# @see http://nginx.org/en/download.html
#
# configure with the distribution configuration
# replace </path/to/> to your real path
#
./configure \
  $(</path/to/>nginx -V 2>&1 | grep 'configure arguments' | sed 's/configure arguments://g') \
  --add-module=</path/to/>ngx_http_google_filter_module \
  --add-module=</path/to/>ngx_http_substitutions_filter_module
#
# if some libraries were missing, you should install them with the package manager
#   eg. apt-get, pacman, yum ...
#
```

#### Usage ####
##### Basic Configuration #####
  `resolver` is needed to resolve domains.
```nginx
server {
  # ... part of server configuration
  resolver 8.8.8.8;
  location / {
    google on;
  }
  # ...
}
```

##### Google Scholar #####
`google_scholar` depends on `google`, so `google_scholar` cannot be used independently.    
Because the tld of google scholar is not the same in different region, so the domain of google scholar should be specific.    
Get the tld of google scholar.
``` bash
curl "scholar.google.com" -I
HTTP/1.1 302 Found
Location: http://scholar.google.co.jp/
```
  Configuration nginx.
``` nginx
location / {
  google on;
  google_scholar "scholar.google.co.jp";
}
```

##### Upstreaming #####
  `upstream` can help you avoid name resolving cost, and decrease the possibility of google robot detection.
``` nginx
upstream www.google.com {
  server 173.194.38.1:443;
  server 173.194.38.2:443;
  server 173.194.38.3:443;
  server 173.194.38.4:443;
}
```

#### Copyright & License ####
  All codes are under [GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt)    
  Copyright (C) 2014 by Cube.

