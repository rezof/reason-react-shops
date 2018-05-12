type state = {
  page: string,
  pages: list(string)
};

type actions =
  | Login
  | Home
  | Account
  | NotFound;

let component = ReasonReact.reducerComponent("App");

let clickHandler = (href, event) =>
  if (! ReactEventRe.Mouse.defaultPrevented(event)) {
    ReactEventRe.Mouse.preventDefault(event);
    ReasonReact.Router.push(href);
  };

let loadToken = () => Dom.Storage.(getItem("token", localStorage));

let make = _children => {
  ...component,
  initialState: () => {
    page: "login",
    pages: ["/login", "/account", "/", "/404"]
  },
  reducer: (action, state) =>
    switch action {
    | Home => ReasonReact.Update({...state, page: "home"})
    | Login => ReasonReact.Update({...state, page: "login"})
    | Account => ReasonReact.Update({...state, page: "account"})
    | NotFound => ReasonReact.Update({...state, page: "404"})
    },
  subscriptions: self => [
    Sub(
      () =>
        ReasonReact.Router.watchUrl(url =>
          switch url.path {
          | ["login"] => self.send(Login)
          | ["account"] => self.send(Account)
          | [] => self.send(Home)
          | _ => self.send(NotFound)
          }
        ),
      ReasonReact.Router.unwatchUrl
    )
  ],
  didMount: _self =>
    switch (loadToken()) {
    | Some(token) =>
      switch token {
      | "" => ReasonReact.Router.push("/login")
      | _ => ReasonReact.Router.push("")
      }
    | _ => ReasonReact.Router.push("/login")
    },
  render: self =>
    <div className="App">
      (
        switch self.state.page {
        | "login" => <Login />
        | "account" => <Account />
        | "home" => <Home />
        | _ => <NotFound />
        }
      )
    </div>
};