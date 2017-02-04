import React, { Component } from 'react';
import Terminal from 'react-bash';
import Pad from 'pad';

export let data = {};
export let GlobalData = {};

export const dodge = {
    exec: ({ structure, history, cwd }, command) => {
        parent.window.Xrm.Utility.openWebResource("skill_loading");
    }
};

export const WhoAmI = {
    exec: ({ structure, history, cwd }, command) => {
        let ret = ["The choosen one!"];
        return { structure, cwd, history: history.concat( ...ret.map(value => ({ value })) )};
    },
};

export const open = {
    exec: ({ structure, history, cwd }, command) => {
        let ret = [];

        if(command.args && command.args[0]) {
            parent.window.Xrm.Utility.openEntityForm("contact", command.args[0], null, {openInNewWindow: true});
        }
        else {
            ret.push("Invalid command use: open [GUID]");
        }

        return { structure, cwd, history: history.concat( ...ret.map(value => ({ value })) )};
    },
};

export const show = {
    exec: ({ structure, history, cwd }, command) => {
        let ret = [];
        let matches = [];

        if(GlobalData["contacts"] == undefined) {
            ret.push("Please hold on... Connecting to the matrix...");
        }
        else if(command.args && command.args[0] == "dreamers") {
            ret.push("Finding dreamers...");

            matches = GlobalData["contacts"].filter((contact) => {
                return contact.preferredcontactmethodcode == 1
            });
        }
        else if(command.args && command.args[0] == "agents") {
            ret.push("Finding agents...");

            matches = GlobalData["contacts"].filter((contact) => {
                return contact.preferredcontactmethodcode == 2
            });
        }
        else {
            ret.push("Invalid command use: show [dreamers|agents]");
        }

        if(matches.length > 0) {
            if(matches.length > 10) {
                ret.push(`OMG, found ${matches.length} matches... Showing first 10: `);
                matches = matches.slice(0, 10);
            }

            matches.forEach(contact => {
                var name = Pad(contact.fullname, 40,".");
                ret.push(`${name} GUID: ${contact.contactid}`);
            })
        }

        return { structure, cwd, history: history.concat( ...ret.map(value => ({ value })) )};
    },
};

const extensions = { dodge, show, open, WhoAmI };

export default class App extends Component {
    constructor(props) {
        super(props);
    }
    
    componentDidMount() {
        console.log("componentDidMount", Pad("test", 100));
        this.props.service.getData().then(contacts => {
            console.log("GOT DATA!!!")
            GlobalData["contacts"] = contacts;
        });
    }

    render() {
        return(<div>
            <Terminal theme="dark" prefix="neo@Nebuchadnezzar" extensions={extensions} />
        </div>);
    }
}
