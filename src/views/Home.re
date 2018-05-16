[%bs.raw "require('../styles/home.css')"];

open Types;

type shops = {
  near: list(shop),
  preferred: list(shop)
};

type resp = {shops};

module Decode = {
  let shops = json : resp =>
    Json.Decode.{
      shops:
        json
        |> field("shops", shops =>
             {
               near:
                 shops
                 |> field(
                      "near",
                      list(item =>
                        {
                          id: item |> field("id", string),
                          name: item |> field("name", string),
                          picture: item |> field("picture", string)
                        }
                      )
                    ),
               preferred:
                 shops
                 |> field(
                      "preferred",
                      list(item =>
                        {
                          id: item |> field("id", string),
                          name: item |> field("name", string),
                          picture: item |> field("picture", string)
                        }
                      )
                    )
             }
           )
    };
};

type state =
  | LOADING_LOCATION
  | LOCATION_LOADED(float, float)
  | LOCATION_FAILED
  | LOADING_SHOPS
  | LOADING_SHOPS_FAILED
  | LOADED(list(shop));

type actions =
  | LOADING_LOCATION
  | LOCATION_LOADED(float, float)
  | LOCATION_FAILED
  | LOADING_SHOPS
  | LOADING_SHOPS_FAILED
  | SHOPS_LOADED(shops);

let component = ReasonReact.reducerComponent("Home");

let getToken = () => {
  let token = Dom.Storage.(getItem("token", localStorage));
  switch token {
  | Some(token) => token
  | _ => ""
  };
};

let fetch_shops = (~longitude, ~latitude) =>
  Fetch.fetchWithInit(
    "http://localhost:4000/api/shops?lantitude="
    ++ string_of_float(latitude)
    ++ "&longitude="
    ++ string_of_float(longitude),
    Fetch.RequestInit.make(
      ~headers=
        Fetch.HeadersInit.make({
          "Accept": "application/json",
          "Authorization": getToken()
        }),
      ()
    )
  );

let make = _children => {
  ...component,
  initialState: () => LOADING_LOCATION,
  reducer: (action, _state) =>
    switch action {
    | LOADING_SHOPS => ReasonReact.Update(LOADING_SHOPS)
    | LOCATION_LOADED(latitude, longitude) =>
      ReasonReact.UpdateWithSideEffects(
        LOADING_SHOPS,
        (
          self =>
            Js.Promise.(
              fetch_shops(~latitude, ~longitude)
              |> then_(Fetch.Response.json)
              |> then_(json => json |> Decode.shops |> resolve)
              |> then_(resp => {
                   self.send(SHOPS_LOADED(resp.shops));
                   resolve(resp.shops);
                 })
              |> catch(err => {
                   Js.log(("error", err));
                   self.send(LOADING_SHOPS_FAILED);
                   reject(Js.Exn.raiseError("failed to load shops"));
                 })
              |> ignore
            )
        )
      )
    | SHOPS_LOADED(shops) => ReasonReact.Update(SHOPS_LOADED(shops))
    | LOADING_SHOPS_FAILED => ReasonReact.Update(LOADING_SHOPS_FAILED)
    | _ => ReasonReact.NoUpdate
    },
  didMount: self => {
    let options: Geolocation.options = {
      enableHighAccuracy: true,
      timeout: 3000,
      maximumAge: 0
    };
    Geolocation.getCurrentPosition(
      options,
      ~success=
        position =>
          self.send(
            LOCATION_LOADED(
              position##coords##latitude,
              position##coords##longitude
            )
          ),
      ~error=error => Js.log(("error", error))
    );
  },
  render: self =>
    switch self.state {
    | LOADING_LOCATION =>
      <p> (ReasonReact.string("getting your location")) </p>
    | LOADING_SHOPS => <p> (ReasonReact.string("loading shops")) </p>
    | SHOPS_LOADED(shops) =>
      <div className="shops-wrapper"> <ShopsList data=shops.near /> </div>
    | LOADING_SHOPS_FAILED =>
      <div> (ReasonReact.string("failed to load shops")) </div>
    | _ => <div> (ReasonReact.string("else")) </div>
    }
};