#! /bin/bash
function envir_check()
{
  [ $UID != 0 ] && echo 必须使用root用户运行脚本 && exit 1
  BUILD=`dpkg -l | grep build-essential`
  if [[ -z $BUILD ]]
  then
    BUILD='NEEDINSTALL'
  else
    BUILD='INSTALLED'
  fi
  GIT=`which git`
  if [[ -z $GIT ]]
  then
  GIT='NEEDINSTALL'
  else
  GIT='INSTALLED'
  fi
  [ -e /opt/nginx-1.8.1/sbin/nginx ] && NGINX='EXIST'
  NGINXPID=`pidof nginx`
}
function install_nginx()
{
  if [[ $BUILD == 'NEEDINSTALL' ]]
  then
    apt-get update
    apt-get install build-essential << EOF
y
EOF
    [ $? != 0 ] && echo build-essential未安装成功,请检查错误输出信息. && exit 2
  fi
  if [[ $GIT == 'NEEDINSTALL' ]]
  then
    apt-get install git << EOF
y
EOF
  [ $? != 0 ] && echo git 未安装成功,请检查错误输出信息. && exit 3
  fi
  [ -e ./nginx-1.8.1.tar.gz ] && tar -zxvf nginx-1.8.1.tar.gz || (wget -t 3 http://nginx.org/download/nginx-1.8.1.tar.gz;tar -zxvf nginx-1.8.1.tar.gz)
  [ $? != 0 ] && echo "nginx 下载失败,详细信息请检查错误输出。" && exit 4
  [ -e ./pcre-8.38.tar.gz ] && tar -zxvf pcre-8.38.tar.gz || (wget -t 3 ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.38.tar.gz;tar -zxvf pcre-8.38.tar.gz)
  [ $? != 0 ] && echo "pcre 下载失败,详细信息请检查错误输出。" && exit 5
  [ -e ./openssl-1.0.2g.tar.gz ] && tar -zxvf openssl-1.0.2g.tar.gz || (wget -t 3 https://www.openssl.org/source/openssl-1.0.2g.tar.gz;tar -zxvf openssl-1.0.2g.tar.gz)
  [ $? != 0 ] && echo "openssl 下载失败,详细信息请检查错误输出。" && exit 6
  [ -e ./zlib-1.2.8.tar.gz ] && tar -zxvf zlib-1.2.8.tar.gz || (wget -t 3 http://zlib.net/zlib-1.2.8.tar.gz;tar -zxvf zlib-1.2.8.tar.gz)
  [ $? != 0 ] && echo "zlib 下载失败,详细信息请检查错误输出。" && exit 7
  git clone https://github.com/cuber/ngx_http_google_filter_module
  [ $? != 0 ] && echo "google module 下载失败,详细信息请检查错误输出。" && exit 8
  git clone https://github.com/yaoweibin/ngx_http_substitutions_filter_module
  [ $? != 0 ] && echo "filter module 下载失败,详细信息请检查错误输出。" && exit 9 
  cd nginx-1.8.1
  ./configure \
  --prefix=/opt/nginx-1.8.1 \
  --with-pcre=../pcre-8.38 \
  --with-openssl=../openssl-1.0.2g \
  --with-zlib=../zlib-1.2.8 \
  --with-http_ssl_module \
  --add-module=../ngx_http_google_filter_module \
  --add-module=../ngx_http_substitutions_filter_module
  [ $? != 0 ] && echo "configure命令执行失败,请检查错误输出。" && exit 10
  make install
  [ $? != 0 ] && echo "安装失败,请检查错误输出。" && exit 11
}
function config_nginx()
{
  cd /opt/nginx-1.8.1/conf
  cat nginx.conf.default | head -n 93 | \
                           sed "s/server_name  localhost/server_name  $domain/g" | \
                           sed 's/#charset koi8-r/return 301 https:\/\/$host$request_uri/g' | \
                           sed 's/#gzip  on/gzip  on/g' > nginx.conf
  cat nginx.conf.default | grep "HTTPS server" -A 12 | \
                           sed "s/server_name  localhost/server_name  $domain/g" | \
                           sed "s#ssl_certificate      cert.pem#ssl_certificate      $cert#g" | \
                           sed "s#ssl_certificate_key  cert.key#ssl_certificate_key  $key#g" | \
                           sed 's/# HTTPS server//g' | \
                           tr -d '#' >> nginx.conf
  echo "resolver 8.8.8.8;" >> nginx.conf
  cat nginx.conf.default | grep "#    ssl_prefer_server_ciphers  on;" -A 8 | \
                           sed 's/#        root   html/google on/g;s/#        index  index.html index.htm;//g' | \
                           tr -d '#' >> nginx.conf
}
echo 注意：仅适用于Debian系操作系统 在Ubuntu14.04中测试通过
echo       不适用Redhat CentOS
echo 为避免风险请不要在运行重要服务的机器里运行这个脚本 脚本作者不负责由此造成的数据丢失等责任 请谨慎使用！
echo "继续吗(yes/no)？"
read con
if [[ $con != "yes" ]]
  then
    echo 脚本退出
    exit 15
fi
envir_check
if [[ -n $NGINXPID ]]
  then
    echo "nginx当前正在运行,要结束程序吗(yes/no)?"
    read com
    if [[ $com == "yes" ]]
      then
        kill $NGINXPID
      else
        echo 脚本退出.
        exit 14
    fi
fi
if [[ $NGINX == "EXIST" ]]
  then
    echo "已安装过nginx,确定重新安装吗(yes/no)?"
    read line
    if [[ $line == "yes" ]]
      then
        rm -rf ./ngx_http_google_filter_module 2> /dev/null
        rm -rf ./ngx_http_substitutions_filter_module 2> /dev/null
        install_nginx
        echo 安装完成 请手动执行/opt/nginx-1.8.1/sbin/nginx
    fi
  else
    echo "请输入您网站证书的绝对路径:"
    read cert
    if [ -e $cert ]
      then
        echo "请输入您证书的私钥绝对路径:"
        read key
        if [ -e $key ]
          then
            echo "请输入您的域名:"
            read domain
            rm -rf ./ngx_http_google_filter_module 2> /dev/null
            rm -rf ./ngx_http_substitutions_filter_module 2> /dev/null
            install_nginx
            config_nginx
            echo "安装完毕,请手动执行/opt/nginx-1.8.1/sbin/nginx,如果您的私钥有密码保护,请输入您的私钥密码。"
        else
          echo "找不到私钥文件,请核对路径后重新运行脚本再次输入。"
          exit 12
        fi
      else
        echo "找不到证书文件,请核对路径后重新运行脚本再次输入。"
        exit 13
    fi
fi
