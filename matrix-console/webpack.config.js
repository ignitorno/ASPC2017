var path = require('path');
var webpack = require('webpack');
var PROD = process.env.NODE_ENV == "production"

console.log("ENV: ", process.env.NODE_ENV);

module.exports = {
  devtool: 'eval',
  entry: [
    'react-hot-loader/patch',
    'webpack-hot-middleware/client',
    './src/index'
  ],
  output: {
    path: path.join(__dirname, 'dist'),
    filename: 'skill_console.js',
  },
  plugins: [
    new webpack.optimize.OccurenceOrderPlugin(),
    new webpack.HotModuleReplacementPlugin(),
    new webpack.NoErrorsPlugin(),
  ],
  module: {
    loaders: [{
      test: /\.jsx?$/,
      loaders: ['babel'],
      include: path.join(__dirname, 'src')
    },{
        test: /\.css$/,
        loader: "style-loader!css-loader",
        include: path.join(__dirname, 'src')
    }]
  }
};

if(PROD) {
  module.exports.entry = [
    './src/index'
  ];

  // No source-map
  module.exports.devtool = undefined;

  module.exports.plugins = [
    // removes a lot of debugging code in React
    new webpack.DefinePlugin({
      'process.env': {
        'NODE_ENV': JSON.stringify('production')
      }
    }),
    new webpack.optimize.UglifyJsPlugin({
        compressor: {
            warnings: false,
            drop_console: true,
            dead_code: true
        },
        output: {
            comments: false
        }
    }),
    new webpack.optimize.DedupePlugin()
  ]
}