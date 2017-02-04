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