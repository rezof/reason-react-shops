[%bs.raw "require('../styles/home.css')"];

open Types;

type shops = {
  near: list(shop),
  preferred: list(shop)
};

type resp = {shops};

module Decoder = {
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

type data =
  | LOADING_LOCATION
  | LOCATION_LOADED(float, float)
  | LOCATION_FAILED
  | LOADING_SHOPS
  | LOADING_SHOPS_FAILED
  | SHOPS_LOADED(shops);

type selected_tab = string;

type state = {
  selected_tab,
  data
};

type action =
  | AC_LOADING_LOCATION
  | AC_LOCATION_LOADED(float, float)
  | AC_LOCATION_FAILED
  | AC_LOADING_SHOPS
  | AC_LOADING_SHOPS_FAILED
  | AC_SHOPS_LOADED(shops)
  | AC_CHANGE_SELECTED_TAB(string);

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
  initialState: () => {data: LOADING_LOCATION, selected_tab: "near"},
  reducer: (action, state) =>
    switch action {
    | AC_LOADING_SHOPS => ReasonReact.Update({...state, data: LOADING_SHOPS})
    | AC_LOCATION_LOADED(latitude, longitude) =>
      ReasonReact.UpdateWithSideEffects(
        {...state, data: LOADING_SHOPS},
        (
          self =>
            Js.Promise.(
              fetch_shops(~latitude, ~longitude)
              |> then_(Fetch.Response.json)
              |> then_(json => json |> Decoder.shops |> resolve)
              |> then_(resp => {
                   self.send(AC_SHOPS_LOADED(resp.shops));
                   resolve(resp.shops);
                 })
              |> catch(err => {
                   Js.log(("error", err));
                   self.send(AC_LOADING_SHOPS_FAILED);
                   reject(Js.Exn.raiseError("failed to load shops"));
                 })
              |> ignore
            )
        )
      )
    | AC_SHOPS_LOADED(shops) =>
      ReasonReact.Update({...state, data: SHOPS_LOADED(shops)})
    | AC_LOADING_SHOPS_FAILED =>
      ReasonReact.Update({...state, data: LOADING_SHOPS_FAILED})
    | AC_CHANGE_SELECTED_TAB(selected_tab) =>
      ReasonReact.Update({...state, selected_tab})
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
            AC_LOCATION_LOADED(
              position##coords##latitude,
              position##coords##longitude
            )
          ),
      ~error=error => Js.log(("error", error))
    );
  },
  render: self =>
    <div className="content-wrapper">
      <Header
        changeSelectedTab=(
          (id: string, _) => self.send(AC_CHANGE_SELECTED_TAB(id))
        )
        selectedTab=self.state.selected_tab
        tabs=[
          {id: "near", text: "Near By Shops"},
          {id: "preferred", text: "My Preferred Shops"}
        ]
      />
      (
        switch self.state.data {
        | LOADING_LOCATION =>
          <p> (ReasonReact.string("getting your location")) </p>
        | LOADING_SHOPS => <p> (ReasonReact.string("loading shops")) </p>
        | SHOPS_LOADED(shops) =>
          switch self.state.selected_tab {
          | "near" =>
            <div className="shops-wrapper">
              <ShopsList data=shops.near />
            </div>
          | "preferred" =>
            <div className="shops-wrapper">
              <ShopsList data=shops.preferred />
            </div>
          | _ => <div> (ReasonReact.string("not a valid tab")) </div>
          }
        | LOADING_SHOPS_FAILED =>
          <div> (ReasonReact.string("failed to load shops")) </div>
        | _ => <div> (ReasonReact.string("else")) </div>
        }
      )
    </div>
};