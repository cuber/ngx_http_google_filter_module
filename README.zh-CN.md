Nginx Google 扩展
=================

[![Build Status](https://travis-ci.org/cuber/ngx_http_google_filter_module.svg?branch=dev)](https://travis-ci.org/cuber/ngx_http_google_filter_module)

#### 扯两句 ####
`wen.lu` 一路走到现在, 离不开大家的支持!
> 很多朋友通过各种方式问过我: "你丫怎么不开源啊..."     
> 先向那些朋友道歉啊, 其实不是我不想开源, 只是之前的版本配置实在太复杂.
> `nginx` 三方扩展用了一大堆, 外加 `lua`, 以及突破千行的配置`工程`, 这么拙劣的技艺, 实在不好意思拿出来分享      

遂决定写一个扩展, 让`google`反代的配置和使用`wen.lu`一样简单.
```nginx
location / {
  google on;
}
```
> _你没有看错, “一行配置, google 我有！”_   
  
#### 现在 [wen.lu](https://wen.lu) 就是由该扩展驱动 ####
![Demo Site](http://ww4.sinaimg.cn/large/68bd1777jw1enbhxn39z8j212q0lu0uo.jpg)
  
#### 依赖库 ####
  1. [`pcre`](http://www.pcre.org/) *正则*
  1. [`ngx_http_proxy_module`](http://nginx.org/en/docs/http/ngx_http_proxy_module.html) *反向代理*
  1. [`ngx_http_substitutions_filter_module`](https://github.com/yaoweibin/ngx_http_substitutions_filter_module) *多重替换*

#### 安装 ####
> **以 ubuntu 14.04 为例** 
> *i386, x86_64 均适用*

##### 最简安装 #####
```bash
#
# 安装 gcc & git
#
apt-get install build-essential git

#
# 下载最新版源码
# nginx 官网: 
# http://nginx.org/en/download.html
#
wget "http://nginx.org/download/nginx-1.7.8.tar.gz"

#
# 下载最新版 pcre
# pcre 官网:
# http://www.pcre.org/
#
wget "ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.36.tar.gz"

#
# 下载最新版 openssl
# opessl 官网:
# https://www.openssl.org/
#
wget "https://www.openssl.org/source/openssl-1.0.1j.tar.gz"

#
# 下载最新版 zlib
# zlib 官网:
# http://www.zlib.net/
#
wget "http://zlib.net/zlib-1.2.8.tar.gz"

#
# 下载本扩展
#
git clone https://github.com/cuber/ngx_http_google_filter_module

#
# 下载 substitutions 扩展
#
git clone https://github.com/yaoweibin/ngx_http_substitutions_filter_module


#
# 解压缩
#
tar xzvf nginx-1.7.8.tar.gz
tar xzvf pcre-8.36.tar.gz
tar xzvf openssl-1.0.1j.tar.gz
tar xzvf zlib-1.2.8.tar.gz

#
# 进入 nginx 源码目录
#
cd nginx-1.7.8

#
# 设置编译选项
#
./configure \
  --prefix=/opt/nginx-1.7.8 \
  --with-pcre=../pcre-8.36 \
  --with-openssl=../openssl-1.0.1j \
  --with-zlib=../zlib-1.2.8 \
  --add-module=../ngx_http_google_filter_module
  --add-module=../ngx_http_substitutions_filter_module
  
#
# 编译, 安装
# 如果扩展有报错, 请发 issue 到
# https://github.com/cuber/ngx_http_google_filter_module/issues
#
make; make install

#
# 启动
#
/opt/nginx-1.7.8/sbin/nginx
```

##### 从发行版迁移 #####
``` bash
#
# 安装 gcc & git
#
apt-get install build-essential git

#
# 安装发行版
# (已安装的请忽略)
#
apt-get install nginx

#
# 查看发行版编译选项及版本
#
nginx -V
# nginx version: nginx/1.4.7
# built by gcc 4.8.2 (Ubuntu 4.8.2-19ubuntu1)
# TLS SNI support enabled
# configure arguments: 
#  --with-cc-opt='-g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2' \
#  --with-ld-opt='-Wl,-Bsymbolic-functions -Wl,-z,relro' \
#  --prefix=/usr/share/nginx \
#  --conf-path=/etc/nginx/nginx.conf \
#  --http-log-path=/var/log/nginx/access.log \
#  --error-log-path=/var/log/nginx/error.log \
#  --lock-path=/var/lock/nginx.lock \
#  --pid-path=/run/nginx.pid \
#  --http-client-body-temp-path=/var/lib/nginx/body \
#  --http-fastcgi-temp-path=/var/lib/nginx/fastcgi \
#  --http-proxy-temp-path=/var/lib/nginx/proxy \
#  --http-scgi-temp-path=/var/lib/nginx/scgi \
#  --http-uwsgi-temp-path=/var/lib/nginx/uwsgi \
#  --with-debug \
#  --with-pcre-jit \
#  --with-ipv6 \
#  --with-http_ssl_module \
#  --with-http_stub_status_module \
#  --with-http_realip_module \
#  --with-http_addition_module \
#  --with-http_dav_module \
#  --with-http_geoip_module \
#  --with-http_gzip_static_module \
#  --with-http_image_filter_module \
#  --with-http_spdy_module \
#  --with-http_sub_module \
#  --with-http_xslt_module \
#  --with-mail \
#  --with-mail_ssl_module

#
# 下载对应 nginx 大版本
# nginx 官网: 
# http://nginx.org/en/download.html
#
wget "http://nginx.org/download/nginx-1.4.7.tar.gz"

#
# 下载本扩展
#
git clone https://github.com/cuber/ngx_http_google_filter_module

#
# 下载 substitutions 扩展
#
git clone https://github.com/yaoweibin/ngx_http_substitutions_filter_module

#
# 安装依赖库的 dev 包
#
apt-get install libpcre3-dev libssl-dev zlib1g-dev libxslt1-dev libgd-dev libgeoip-dev

./configure \
  --with-cc-opt='-g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2' \
  --with-ld-opt='-Wl,-Bsymbolic-functions -Wl,-z,relro' \
  --prefix=/usr/share/nginx \
  --conf-path=/etc/nginx/nginx.conf \
  --http-log-path=/var/log/nginx/access.log \
  --error-log-path=/var/log/nginx/error.log \
  --lock-path=/var/lock/nginx.lock \
  --pid-path=/run/nginx.pid \
  --http-client-body-temp-path=/var/lib/nginx/body \
  --http-fastcgi-temp-path=/var/lib/nginx/fastcgi \
  --http-proxy-temp-path=/var/lib/nginx/proxy \
  --http-scgi-temp-path=/var/lib/nginx/scgi \
  --http-uwsgi-temp-path=/var/lib/nginx/uwsgi \
  --with-debug \
  --with-pcre-jit \
  --with-ipv6 \
  --with-http_ssl_module \
  --with-http_stub_status_module \
  --with-http_realip_module \
  --with-http_addition_module \
  --with-http_dav_module \
  --with-http_geoip_module \
  --with-http_gzip_static_module \
  --with-http_image_filter_module \
  --with-http_spdy_module \
  --with-http_sub_module \
  --with-http_xslt_module \
  --with-mail \
  --with-mail_ssl_module \
  --add-module=../ngx_http_google_filter_module \
  --add-module=../ngx_http_substitutions_filter_module
  
#
# 覆盖二进制文件
#
cp -rf objs/nginx /usr/sbin/nginx

#
# 重启 nginx
# 
service nginx stop
service nginx start
```

#### 配置方法 ####
##### 基本搜索 #####
  需要配置 `resolver` 用于域名解析
```nginx
server {
  # ... 仅列举部分配置
  resolver 8.8.8.8;
  location / {
    google on;
  }
  # ...
}
```

##### 谷歌学术 #####
`google_scholar` 依赖于 `google`, 所以 `google_scholar` 无法独立使用.    
由于不同区域的谷歌学术顶级域不同, 所以需要明确配置谷歌学术的域名.
``` bash
# 
# 获取谷歌学术的域名的方法
#
curl "scholar.google.com" -I
HTTP/1.1 302 Found
Location: http://scholar.google.co.jp/
```
  配置 nginx
``` nginx
location / {
  google on;
  google_scholar "scholar.google.co.jp";
}
```

##### 默认语言偏好 #####
默认的语言偏好可用 `google_language` 来设置, 如果没有设置, 默认使用 `zh-CN` (中文)
```nginx
location / {
  google on;
  google_scholar "scholar.google.co.jp";
  # 设置成德文
  google_language "de"; 
}
```

支持的语言如下.
```txt
de en es es-419 fr hr it nl pl pt-BR pt-PT 
vi tr ru ar th ko zh-CN zh-TW ja
```

##### Upstreaming #####
`upstream` 减少一次域名解析的开销, 并且通过配置多个网段的 google ip 能够一定程度上减少被 google 机器人识别程序侦测到的几率 (弹验证码).
``` nginx
upstream www.google.com {
  server 173.194.38.1:443;
  server 173.194.38.2:443;
  server 173.194.38.3:443;
  server 173.194.38.4:443;
}
```

##### Proxy Protocal #####
默认采用 `https` 与后端服务器通信.    
你可以使用 `google_ssl_off` 来强制将一些域名降到 `http` 协议.    
如果你想让, 这个设置可以让一些通过跳板机二次代理的域名用 `http` 协议进行转发, 从而可以不再依赖 `ssl` 证书.
```nginx
#
# 例如 'www.googl.com' 按如下方式代理
# vps(hk) -> vps(us) -> google
#

#
# vps(hk) 配置
#
server {
  # ...
  location / {
    google on;
    google_ssl_off "www.google.com";
  }
  # ...
}

upstream www.google.com {
  server < vps(hk) 的 ip >:80;
}

#
# vps(us) 配置
#
server {
  listen 80;
  server_name www.google.com;
  # ...
  location / {
    proxy_pass https://www.google.com;
  }
  # ...
}
```

#### Copyright & License ####
  所有代码都遵循 [GPLv2](http://www.gnu.org/licenses/gpl-2.0.txt) 开源协议   
  Copyright (C) 2014 by Cube.



