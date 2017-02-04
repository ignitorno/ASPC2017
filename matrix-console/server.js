var path = require('path');
var webpack = require('webpack');
var express = require('express');
var https = require('https');
var http = require('http');
var devMiddleware = require('webpack-dev-middleware');
var hotMiddleware = require('webpack-hot-middleware');
var config = require('./webpack.config');
var fs = require('fs');

var app = express();
var compiler = webpack(config);

app.use(devMiddleware(compiler, {
  publicPath: config.output.publicPath,
  historyApiFallback: true
}));

app.use(hotMiddleware(compiler));

app.get('*', function (req, res) {
  res.sendFile(path.join(__dirname, 'dist/skill_console.html'));
});

// http.createServer(app).listen(3000, null,null, function() {
//   console.log('Server listening on port %d in %s mode', this.address().port, app.settings.env);
// });

// chrome://flags/#allow-insecure-localhost

var options = {
    key  : fs.readFileSync('ssl/key.pem'),
    ca   : fs.readFileSync('ssl/csr.pem'),
    cert : fs.readFileSync('ssl/cert.pem')
}

https.createServer(options, app).listen(3000, null,null, function() {
  console.log('Server listening on port %d in %s mode', this.address().port, app.settings.env);
  console.log(this.address());
});
