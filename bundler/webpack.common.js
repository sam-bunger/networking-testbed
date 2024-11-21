/* eslint-disable @typescript-eslint/no-var-requires */
const CopyWebpackPlugin = require('copy-webpack-plugin')
const HtmlWebpackPlugin = require('html-webpack-plugin')
const webpack = require('webpack')
const path = require('path')
const version = require('../package.json').version

const TARGET_NUM_BUNDLES = 2

module.exports = {
  entry: {
    main: path.resolve(__dirname, '../src/index.tsx'),
  },
  output: {
    filename: '[name].[contenthash].js',
    chunkFilename: '[id].[contenthash].js',
    path: path.resolve(__dirname, '../dist'),
  },
  plugins: [
    new CopyWebpackPlugin({
      patterns: [
        {
          from: path.resolve(__dirname, '../static'),
          noErrorOnMissing: true,
        },
      ],
    }),
    new HtmlWebpackPlugin({
      template: path.resolve(__dirname, '../src/index.html'),
    }),
    new webpack.DefinePlugin({
      'process.env.VERSION': `"${version}"`,
      'process.env.CLOUDFLARE_ENV': JSON.stringify(process.env.ENV),
    }),
  ],
  resolve: {
    extensions: ['.tsx', '.ts', '.js', '.jsx'],
    alias: {
      three$: path.resolve(__dirname, '../node_modules/three/src/Three.js'),
      three: path.resolve(__dirname, '../node_modules/three/'),
      assets: path.resolve(__dirname, '../node_modules/assets'),
      '@': path.resolve(__dirname, '../src'),
    },
    fallback: {
      fs: false,
      path: false,
      perf_hooks: false,
      worker_threads: false,
      crypto: false,
    },
  },
  externals: {
    'node:buffer': 'ignore',
    'node:url': 'ignore',
    'jsdom-global': 'ignore',
  },
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        exclude: /node_modules/,
        use: [
          {
            loader: 'babel-loader',
            options: {
              presets: [
                [
                  '@babel/preset-env',
                  {
                    useBuiltIns: 'entry',
                    corejs: { version: '3.32', proposals: true },
                    modules: false,
                  },
                ],
                '@babel/preset-react',
                '@babel/preset-typescript',
              ],
              plugins: [
                '@babel/plugin-proposal-explicit-resource-management',
                ['@babel/plugin-proposal-decorators', { legacy: true }],
                '@babel/plugin-proposal-class-properties',
              ],
            },
          },
        ],
      },
      {
        test: /\.(html)$/,
        use: 'html-loader',
      },
      {
        test: /\.(jpg|jpeg|png|gif|svg|webp)$/,
        type: 'asset/resource',
      },
      {
        test: /\.(wasm|worker\.js|wasmwrapper\.js)$/,
        type: 'asset/resource',
      },
      {
        test: /\.(glb|gltf|fbx|obj|drift|bin)$/,
        type: 'asset/resource',
      },
      {
        test: /\.(mp3|webm)$/,
        type: 'asset/resource',
      },
      {
        test: /\.(ttf|eot|woff|woff2|otf)$/,
        type: 'asset/resource',
      },
      {
        test: /\.(s[ac]ss|css)$/,
        use: ['style-loader', 'css-loader', 'sass-loader'],
      },
    ],
  },
  optimization: {
    runtimeChunk: 'single',
    splitChunks: {
      chunks: 'all',
      maxInitialRequests: Infinity,
      minSize: 0,
      cacheGroups: {
        vendor: {
          test(module) {
            // Adjust this to identify your vendor modules correctly
            return module.context && !module.context.includes('src')
          },
          name(module) {
            // Default fallback
            let packageName = 'vendor'

            if (module.context) {
              const match = module.context.match(/\.pnpm\/(.*?)\//)
              if (match) {
                packageName = match[1].replace('@', '').replace('+', '.').replace('/', '.')
              }
            }

            if (packageName === 'vendor') {
              return 'vendor.internal'
            }

            // Hash the package name and use modulo to determine the bundle it should belong to.
            const hash = packageName.split('').reduce((acc, char) => {
              return char.charCodeAt(0) + ((acc << 5) - acc)
            }, 0)

            // Derive the bundle number from the hash.
            const bundleNumber = Math.abs(hash) % TARGET_NUM_BUNDLES

            return `vendor.bundle${bundleNumber}`
          },
        },
      },
    },
  },
}
