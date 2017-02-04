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
        // Preload data for faster access
        this.props.service.getData().then(contacts => {
            window.GlobalData["contacts"] = contacts;
        });
    }

    render() {
        // Render terminal with our extension commands
        return(<div>
            <Terminal theme="dark" prefix="neo@Nebuchadnezzar" extensions={extensions} />
        </div>);
    }
}
