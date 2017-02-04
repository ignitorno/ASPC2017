import { AppContainer } from 'react-hot-loader';
import React from 'react';
import ReactDOM from 'react-dom';
import App from './components/App';
import Service from './Service';

//import CreditService from './service/CreditService'
//import CrmWrapper from './service/CrmWrapper'

require('./style.css')

const service = new Service();

//window.creditDev = {creditService: creditService, crmWrapper: CrmWrapper};

const rootEl = document.getElementById('foreground');
ReactDOM.render(
  <AppContainer>
    <App service={service} />
  </AppContainer>,
  rootEl
);

if (module.hot) {
  module.hot.accept('./components/App', () => {
    // If you use Webpack 2 in ES modules mode, you can
    // use <App /> here rather than require() a <NextApp />.
    const NextApp = require('./components/App').default;
    
    ReactDOM.render(
      <AppContainer>
         <NextApp service={service} />
      </AppContainer>,
      rootEl
    );
  });
}

//module.hot.accept();