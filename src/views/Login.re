[%bs.raw "require('../styles/login.css')"];

type errors = list(string);

type response = {
  success: bool,
  token: option(string),
  errors: option(errors)
};

module Decode = {
  let response = json : response =>
    Json.Decode.{
      success: json |> field("success", bool),
      token: json |> optional(field("token", string)),
      errors: json |> optional(field("errors", list(string)))
    };
};

type state = {
  errors,
  email: string,
  password: string,
  loading: bool
};

type actions =
  | Login(string, string)
  | EmailInputChange(string)
  | PassInputChange(string)
  | LoginSuccessful(option(string))
  | LoginFailed(option(errors))
  | UnknownError;

let component = ReasonReact.reducerComponent("Login");

let fieldValue = event => ReactDOMRe.domElementToObj(
                            ReactEventRe.Form.target(event)
                          )##value;

let loginHandler = (event, self) => {
  if (! ReactEventRe.Form.isDefaultPrevented(event)) {
    ReactEventRe.Form.preventDefault(event);
  };
  let email = self.ReasonReact.state.email;
  let password = self.ReasonReact.state.password;
  self.send(Login(email, password));
  ();
};

let login = payload =>
  Fetch.fetchWithInit(
    "http://localhost:4000/api/login",
    Fetch.RequestInit.make(
      ~method_=Post,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(Js.Json.object_(payload))),
      ~headers=
        Fetch.HeadersInit.make({
          "Content-Type": "application/json",
          "Accept": "application/json"
        }),
      ()
    )
  );

let make = _children => {
  ...component,
  initialState: () => {email: "", password: "", errors: [], loading: false},
  reducer: (action, state) =>
    switch action {
    | EmailInputChange(value) => ReasonReact.Update({...state, email: value})
    | PassInputChange(value) => ReasonReact.Update({...state, password: value})
    | LoginSuccessful(token) =>
      switch token {
      | Some(token) =>
        Dom.Storage.(setItem("token", token, localStorage));
        ReasonReact.Router.push("/");
        ReasonReact.NoUpdate;
      | _ => ReasonReact.NoUpdate
      }
    | LoginFailed(errors) =>
      switch errors {
      | Some(errors) => ReasonReact.Update({...state, errors})
      | _ => ReasonReact.NoUpdate
      }
    | Login(email, password) =>
      ReasonReact.UpdateWithSideEffects(
        {...state, loading: true},
        (
          self => {
            let payload = Js.Dict.empty();
            Js.Dict.set(payload, "email", Js.Json.string(email));
            Js.Dict.set(payload, "password", Js.Json.string(password));
            Js.Promise.(
              login(payload)
              |> then_(Fetch.Response.json)
              |> then_(resp => Js.Promise.resolve(Decode.response(resp)))
              |> then_(resp =>
                   switch resp.success {
                   | true => resolve(self.send(LoginSuccessful(resp.token)))
                   | false => resolve(self.send(LoginFailed(resp.errors)))
                   | _ => resolve(self.send(UnknownError))
                   }
                 )
              |> ignore
            );
          }
        )
      )
    | _ => ReasonReact.NoUpdate
    },
  render: self =>
    <div className="wrapper">
      <div className="form-wrapper">
        <h1 className="title"> (ReasonReact.string("Login")) </h1>
        <form onSubmit=(event => loginHandler(event, self))>
          (
            self.state.errors
            |> Array.of_list
            |> Array.mapi((i, error) =>
                 <p key=(string_of_int(i)) className="login-error">
                   (ReasonReact.string(error))
                 </p>
               )
            |> ReasonReact.array
          )
          <div className="input-wrapper">
            <input
              _type="email"
              placeholder="test@email.co"
              onChange=(
                event => self.send(EmailInputChange(fieldValue(event)))
              )
              value=self.state.email
            />
          </div>
          <div className="input-wrapper">
            <input
              _type="password"
              placeholder="******"
              onChange=(event => self.send(PassInputChange(fieldValue(event))))
              value=self.state.password
            />
          </div>
          <div className="input-wrapper">
            <input _type="submit" value="Login" />
          </div>
        </form>
      </div>
    </div>
};