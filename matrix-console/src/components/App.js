import React, { Component } from 'react';
import Terminal from 'react-bash';


import {show, dodge, open, WhoAmI} from './Commands';

export let data = {};
window.GlobalData = {};

const extensions = { dodge, show, open, WhoAmI };

export default class App extends Component {
    constructor(props) {
        super(props);
    }
    
    componentDidMount() {
        this.props.service.getData().then(contacts => {
            window.GlobalData["contacts"] = contacts;
        });
    }

    render() {
        return(<div>
            <Terminal theme="dark" prefix="neo@Nebuchadnezzar" extensions={extensions} />
        </div>);
    }
}
