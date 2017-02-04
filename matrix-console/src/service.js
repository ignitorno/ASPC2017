export default class Service {
    show(value) {
        console.log("SHOW", value);
    }
    getData() {
        console.log("preloading data");
        const action = "contacts/?$select=fullname,preferredcontactmethodcode";

        return this._fetch(action).then(json => {
            return json.value;
        });      
    }

    _fetch(action) {
        const xrmClientUrl = "https://skillaspc2017.crm4.dynamics.com";
        
        const absoluteUrl = `${xrmClientUrl}/api/data/v8.2/${action}`;

        return fetch(absoluteUrl, {
            credentials: 'same-origin',
            headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json; charset=utf-8',
                'OData-Version': '4.0',
                'OData-MaxVersion': '4.0',
                'Prefer': 'odata.include-annotations="*"'
            }
        }).then(response => {
            if(response.ok == false) {
                throw new Error(`fetch ${url} failed.\nServer returned: ${response.status} ${response.statusText}`);        
            }
            return response.json();
        })
    }    
}